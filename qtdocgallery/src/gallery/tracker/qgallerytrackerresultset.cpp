/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtDocGallery module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <tracker-sparql.h>

#include "qgallerytrackerresultset_p_p.h"

#include "qgallerytrackermetadataedit_p.h"

#include <QtCore/qdatetime.h>
#include <QtDBus/qdbusreply.h>

#include <qdocumentgallery.h>
#include <qgalleryresource.h>

#include "../../3rdparty/synchronization/synchronizelists.h"

namespace {

class SynchronizationItem
{
public:
    SynchronizationItem(QVector<QVariant>::const_iterator begin, int width)
        : begin(begin)
        , end(begin + width)
    {
    }

    QVector<QVariant>::const_iterator begin;
    QVector<QVariant>::const_iterator end;
};

class SynchronizationList
{
public:
    SynchronizationList(const QVector<QVariant> &values, int tableWidth, int identityWidth)
        : m_values(values)
        , m_tableWidth(tableWidth)
        , m_identityWidth(identityWidth)
        , m_count(m_values.count() / tableWidth)
    {
    }

    typedef const SynchronizationItem const_reference;

    SynchronizationItem at(int index) const {
        return SynchronizationItem(m_values.begin() + index * m_tableWidth, m_identityWidth); }
    int count() const { return m_count; }

private:
    const QVector<QVariant> &m_values;
    const int m_tableWidth;
    const int m_identityWidth;
    const int m_count;
};

}

typedef QT_DOCGALLERY_PREPEND_NAMESPACE(QGalleryTrackerResultSetPrivate::SyncEvent) SyncEvent;

template <>
bool compareIdentity<SynchronizationItem>(const SynchronizationItem &item, const SynchronizationItem &reference)
{
    for (QVector<QVariant>::const_iterator iit = item.begin, rit = reference.begin;
            iit != item.end;
            ++iit, ++rit) {
        if (*iit != *rit)
            return false;
    }
    return true;
}

template <>
int insertRange<QVector<SyncEvent>, SynchronizationList>(
        QVector<SyncEvent> *events, int index, int count, const SynchronizationList &, int sourceIndex)
{
    events->append(SyncEvent(SyncEvent::Insert, index, count, sourceIndex));

    return 0;
}

template <>
int removeRange<QVector<SyncEvent> >(QVector<SyncEvent> *events, int index, int count)
{
    events->append(SyncEvent(SyncEvent::Remove, index, count));

    return count;
}

template <>
int updateRange<QVector<SyncEvent>, SynchronizationList>(
        QVector<SyncEvent> *events, int index, int count, const SynchronizationList &, int sourceIndex)
{
    events->append(SyncEvent(SyncEvent::Update, index, count, sourceIndex));

    return count;
}

QT_BEGIN_NAMESPACE_DOCGALLERY

void QGalleryTrackerResultSetPrivate::update()
{
    flags &= ~UpdateRequested;

    updateTimer.stop();

    typedef QList<QGalleryTrackerMetaDataEdit *>::iterator iterator;
    for (iterator it = edits.begin(), end = edits.end(); it != end; ++it)
        (*it)->commit();
    edits.clear();

    if (!(flags & (Active | Cancelled))) {
        query();

        flags &= ~Refresh;
    }
}

void QGalleryTrackerResultSetPrivate::query()
{
    flags &= ~(Refresh | SyncFinished);
    flags |= Active;

    updateTimer.stop();

    parserThread.start(QThread::LowPriority);

    emit q_func()->progressChanged(progressMaximum - 1, progressMaximum);
}

void QGalleryTrackerResultSetPrivate::run()
{
    QVector<QVariant> cacheValues = values;
    cacheValues.detach();

    QVector<QVariant> referenceValues;
    referenceValues.reserve(100 * tableWidth);

    const SynchronizationList cache(cacheValues, tableWidth, identityWidth);

    int cacheIndex = 0;
    int referenceIndex = 0;

    GError *error = 0;
    if (TrackerSparqlCursor *cursor = tracker_sparql_connection_query(
                connection, sparql.toUtf8(), 0, &error)) {
        const QVariant variant;

        for (bool atEnd = false; !atEnd;) {
            for (int batchItem = 0; batchItem < 500; ++batchItem) {
                if (!tracker_sparql_cursor_next(cursor, 0, 0)) {
                    atEnd = true;
                    break;
                }

                const int rowWidth = qMin(tableWidth, tracker_sparql_cursor_get_n_columns(cursor));
                int i = 0;
                for (; i < rowWidth; ++i) {
                    referenceValues.append(valueColumns.at(i)->toVariant(cursor, i));
                }
                for (; i < tableWidth; ++i)
                    referenceValues.append(variant);
            }

            QVector<SyncEvent> events;
            const SynchronizationList reference(referenceValues, tableWidth, identityWidth);

            synchronizeList(&events, cache, cacheIndex, reference, referenceIndex);

            QMutexLocker locker(&mutex);
            syncValues = referenceValues;

            if (syncEvents.isEmpty() && !events.isEmpty())
                QCoreApplication::postEvent(q_func(), new QEvent(QEvent::UpdateLater));

            syncEvents += events;
        }
        g_object_unref(G_OBJECT(cursor));
    } else {
        queryError = QDocumentGallery::FilterError;
        queryErrorString = QString::fromUtf8(error->message);
        g_error_free(error);
    }

    QVector<SyncEvent> events;
    const SynchronizationList reference(referenceValues, tableWidth, identityWidth);
    completeSynchronizeList(&events, cache, cacheIndex, reference, referenceIndex);

    QMutexLocker locker(&mutex);

    syncValues = referenceValues;

    if (syncEvents.isEmpty())
        QCoreApplication::postEvent(q_func(), new QEvent(QEvent::UpdateLater));

    syncEvents += events;
    syncEvents.append(SyncEvent(SyncEvent::Finish, 0, 0));
}

void QGalleryTrackerResultSetPrivate::processSyncEvents()
{
    QMutexLocker locker(&mutex);

    QVector<SyncEvent> events = syncEvents;
    syncEvents.clear();

    QVector<QVariant> eventValues = syncValues;

    locker.unlock();

    foreach (const SyncEvent &event, events) {
        switch (event.type) {
        case SyncEvent::Insert: {
            const int index = event.index + syncDelta;
            syncDelta += event.count;
            rowCount += event.count;

            if (currentIndex >= index)
                currentIndex += event.count;

            values = values.mid(0, index * tableWidth)
                    + eventValues.mid(event.sourceIndex * tableWidth, event.count * tableWidth)
                    + values.mid(index * tableWidth);

            emit q_func()->itemsInserted(index, event.count);
            break;
        }
        case SyncEvent::Remove: {
            bool removedCurrent = false;
            const int index = event.index + syncDelta;
            syncDelta -= event.count;
            rowCount -= event.count;

            if (currentIndex >= index + event.count) {
                currentIndex -= event.count;
            } else if (currentIndex >= index) {
                currentIndex = -1;
                removedCurrent = true;
            }

            values.remove(index * tableWidth, event.count * tableWidth);
            emit q_func()->itemsRemoved(index, event.count);

            if (removedCurrent)
                emit q_func()->currentItemChanged();
            break;
        }
        case SyncEvent::Update: {
            const int index = event.index + syncDelta;

            values = values.mid(0, index * tableWidth)
                    + eventValues.mid(event.sourceIndex * tableWidth, event.count * tableWidth)
                    + values.mid((index + event.count) * tableWidth);

            emit q_func()->metaDataChanged(index, event.count, propertyKeys);
            break;
        }
        case SyncEvent::Finish: {
            flags |= SyncFinished;
            syncDelta = 0;
            break;
        }
        default:
            break;
        }
    }
}

bool QGalleryTrackerResultSetPrivate::waitForSyncFinish(int msecs)
{
    QTime timer;
    timer.start();

    do {
        processSyncEvents();

        if (flags & SyncFinished) {
            return true;
        }

        if (!parserThread.wait(msecs))
            return false;
    } while ((msecs -= timer.restart()) > 0);

    return false;
}

void QGalleryTrackerResultSetPrivate::_q_parseFinished()
{
    processSyncEvents();

    flags &= ~Active;

    if (flags & Refresh)
        update();
    else
        emit q_func()->progressChanged(progressMaximum, progressMaximum);

    if (queryError != QDocumentGallery::NoError) {
        q_func()->finish(flags & Live);
    } else  {
        q_func()->error(queryError, queryErrorString);
        queryError = QDocumentGallery::NoError;
    }
}

void QGalleryTrackerResultSetPrivate::_q_editFinished(QGalleryTrackerMetaDataEdit *edit)
{
    edit->deleteLater();

    emit q_func()->itemEdited(m_service);
}

QGalleryTrackerResultSet::QGalleryTrackerResultSet(
        TrackerSparqlConnection *connection,
        QGalleryTrackerResultSetArguments *arguments,
        bool autoUpdate,
        QObject *parent)
    : QGalleryResultSet(*new QGalleryTrackerResultSetPrivate(connection, arguments, autoUpdate), parent)
{
    Q_D(QGalleryTrackerResultSet);

    g_object_ref(G_OBJECT(d->connection));

    connect(&d->parserThread, SIGNAL(finished()), this, SLOT(_q_parseFinished()));

    d_func()->query();
}

QGalleryTrackerResultSet::QGalleryTrackerResultSet(
        QGalleryTrackerResultSetPrivate &dd,
        QObject *parent)
    : QGalleryResultSet(dd, parent)
{
    Q_D(QGalleryTrackerResultSet);

    g_object_ref(G_OBJECT(d->connection));

    connect(&d->parserThread, SIGNAL(finished()), this, SLOT(_q_parseFinished()));

    d_func()->query();
}

QGalleryTrackerResultSet::~QGalleryTrackerResultSet()
{
    Q_D(QGalleryTrackerResultSet);

    typedef QList<QGalleryTrackerMetaDataEdit *>::iterator iterator;
    for (iterator it = d->edits.begin(), end = d->edits.end(); it != end; ++it)
        (*it)->commit();

    d->parserThread.wait();

    g_object_unref(G_OBJECT(d->connection));
}

QStringList QGalleryTrackerResultSet::propertyNames() const
{
    return d_func()->propertyNames;
}

int QGalleryTrackerResultSet::propertyKey(const QString &property) const
{
    Q_D(const QGalleryTrackerResultSet);

    int index = d->propertyNames.indexOf(property);

    return index >= 0
            ? index + d->valueOffset
            : -1;
}

QGalleryProperty::Attributes QGalleryTrackerResultSet::propertyAttributes(int key) const
{
    return d_func()->propertyAttributes.value(key - d_func()->valueOffset);
}

QVariant::Type QGalleryTrackerResultSet::propertyType(int key) const
{
    return d_func()->propertyTypes.value(key - d_func()->valueOffset);
}

int QGalleryTrackerResultSet::itemCount() const
{
    return d_func()->rowCount;
}

int QGalleryTrackerResultSet::currentIndex() const
{
    return d_func()->currentIndex;
}

bool QGalleryTrackerResultSet::fetch(int index)
{
    Q_D(QGalleryTrackerResultSet);

    d->currentIndex = index >= 0 && index < d->rowCount
            ? index
            : -1;

    emit currentIndexChanged(d->currentIndex);
    emit currentItemChanged();

    return d->currentIndex != -1;
}

QVariant QGalleryTrackerResultSet::itemId() const
{
    Q_D(const QGalleryTrackerResultSet);

    return d->currentIndex >= 0
            ? d->idColumn->value(d->currentRow())
            : QVariant();
}

QUrl QGalleryTrackerResultSet::itemUrl() const
{
    Q_D(const QGalleryTrackerResultSet);

    return d->currentIndex >= 0
            ? d->urlColumn->value(d->currentRow()).toUrl()
            : QUrl();
}

QString QGalleryTrackerResultSet::itemType() const
{
    Q_D(const QGalleryTrackerResultSet);

    return d->currentIndex >= 0
            ? d->typeColumn->value(d->currentRow()).toString()
            : QString();
}

QList<QGalleryResource> QGalleryTrackerResultSet::resources() const
{
    Q_D(const QGalleryTrackerResultSet);

    QList<QGalleryResource> resources;

    if (d->currentIndex >= 0) {
        const QUrl url = d->urlColumn->value(d->currentRow()).toUrl();

        if (!url.isEmpty()) {
            QMap<int, QVariant> attributes;

            typedef QVector<int>::const_iterator iterator;
            for (iterator it = d->resourceKeys.begin(), end = d->resourceKeys.end();
                    it != end;
                    ++it) {
                QVariant value = metaData(*it);

                if (!value.isNull())
                    attributes.insert(*it, value);
            }

            resources.append(QGalleryResource(url, attributes));
        }
    }
    return resources;
}

QVariant QGalleryTrackerResultSet::metaData(int key) const
{
    Q_D(const QGalleryTrackerResultSet);

    if (d->currentIndex == -1 || key < d->valueOffset) {
        return QVariant();
    } else if (key < d->compositeOffset) {  // Value column.
        return *(d->currentRow() + key);
    } else if (key < d->aliasOffset) {      // Composite column.
        return d->compositeColumns.at(key - d->compositeOffset)->value(d->currentRow());
    } else if (key < d->columnCount) {      // Alias column.
        return *(d->currentRow() + d->aliasColumns.at(key - d->aliasOffset) + d->valueOffset);
    } else {
        return QVariant();
    }
}

bool QGalleryTrackerResultSet::setMetaData(int, const QVariant &)
{
    return false;
}

void QGalleryTrackerResultSet::cancel()
{
    d_func()->flags |= QGalleryTrackerResultSetPrivate::Cancelled;
    d_func()->flags &= ~QGalleryTrackerResultSetPrivate::Live;

    if (!(d_func()->flags &QGalleryTrackerResultSetPrivate::Active))
        QGalleryAbstractResponse::cancel();
}

bool QGalleryTrackerResultSet::waitForFinished(int msecs)
{
    Q_D(QGalleryTrackerResultSet);

    QTime timer;
    timer.start();

    do {
        if (d->flags & QGalleryTrackerResultSetPrivate::Active) {
            if (d->waitForSyncFinish(msecs)) {
                d->parserThread.wait();

                d->_q_parseFinished();

                if (!(d->flags & QGalleryTrackerResultSetPrivate::Active))
                    return true;
            } else {
                return false;
            }
        } else if (d->flags & (QGalleryTrackerResultSetPrivate::Refresh)) {
            d->update();
        } else {
            return true;
        }
    } while ((msecs -= timer.restart()) > 0);

    return false;
}

bool QGalleryTrackerResultSet::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::UpdateRequest:
        d_func()->update();

        return true;
    case QEvent::UpdateLater:
        d_func()->processSyncEvents();

        return true;
    default:
        return QGalleryAbstractResponse::event(event);
    }
}

void QGalleryTrackerResultSet::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == d_func()->updateTimer.timerId()) {
        d_func()->update();

        event->accept();
   }
}

void QGalleryTrackerResultSet::refresh(int updateId)
{
    Q_D(QGalleryTrackerResultSet);

    if ((d->updateMask & updateId)
            && !d->updateTimer.isActive()
            && (d->flags & QGalleryTrackerResultSetPrivate::Live)) {


        d->flags |= QGalleryTrackerResultSetPrivate::Refresh;

        if (!(d->flags & QGalleryTrackerResultSetPrivate::Active)) {
            d->updateTimer.start(100, this);
        }
    }
}

QT_END_NAMESPACE_DOCGALLERY

#include "moc_qgallerytrackerresultset_p.cpp"
