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

#include "qgallerytrackereditableresultset_p.h"

#include "qgallerytrackerresultset_p_p.h"
#include "qgallerytrackerschema_p.h"

#include <QtCore/qcoreapplication.h>
#include <QtDBus/qdbuspendingreply.h>

Q_DECLARE_METATYPE(QVector<QStringList>)

QT_BEGIN_NAMESPACE_DOCGALLERY

class QGalleryTrackerEditableResultSetPrivate : public QGalleryTrackerResultSetPrivate
{
    Q_DECLARE_PUBLIC(QGalleryTrackerEditableResultSet)
public:
    QGalleryTrackerEditableResultSetPrivate(
            TrackerSparqlConnection *connection,
            QGalleryTrackerResultSetArguments *arguments,
            const QGalleryDBusInterfacePointer &metaDataInterface,
            bool autoUpdate)
        : QGalleryTrackerResultSetPrivate(connection, arguments, autoUpdate)
        , metaDataInterface(metaDataInterface)
        , fieldNames(arguments->fieldNames)
    {
    }

    const QGalleryDBusInterfacePointer metaDataInterface;
    const QStringList fieldNames;
};

QGalleryTrackerEditableResultSet::QGalleryTrackerEditableResultSet(
        TrackerSparqlConnection *connection,
        QGalleryTrackerResultSetArguments *arguments,
        const QGalleryDBusInterfacePointer &metaDataInterface,
        bool autoUpdate,
        QObject *parent)
    : QGalleryTrackerResultSet(
            *new QGalleryTrackerEditableResultSetPrivate(
                connection, arguments, metaDataInterface, autoUpdate),
            parent)
{
}

QGalleryTrackerEditableResultSet::~QGalleryTrackerEditableResultSet()
{
}

bool QGalleryTrackerEditableResultSet::setMetaData(int key, const QVariant &value)
{
    Q_D(QGalleryTrackerEditableResultSet);

    if (d->currentIndex == -1 || key < d->valueOffset || key >= d->columnCount)
        return false;
    else if (key >= d->aliasOffset)
        key = d->aliasColumns.at(key - d->aliasOffset) + d->valueOffset;

    if (key >= d->compositeOffset)
        return false;

    if (*(d->currentRow() + key) == value)
        return true;

    QGalleryTrackerMetaDataEdit *edit = 0;

    typedef QList<QGalleryTrackerMetaDataEdit *>::iterator iterator;
    for (iterator it = d->edits.begin(), end = d->edits.end(); it != end; ++it) {
        if ((*it)->index() == d->currentIndex) {
            edit = *it;
            break;
        }
    }

    if (!edit) {
        edit = new QGalleryTrackerMetaDataEdit(
                d->metaDataInterface,
                (d->currentRow() + 1)->toString(),
                d->currentRow()->toString(),
                this);
        edit->setIndex(d->currentIndex);

        connect(edit, SIGNAL(finished(QGalleryTrackerMetaDataEdit*)),
                this, SLOT(_q_editFinished(QGalleryTrackerMetaDataEdit*)));

        connect(this, SIGNAL(itemsInserted(int,int)), edit, SLOT(itemsInserted(int,int)));
        connect(this, SIGNAL(itemsRemoved(int,int)), edit, SLOT(itemsRemoved(int,int)));

        d->edits.append(edit);

        d->requestUpdate();
    }

    edit->setValue(
            d->fieldNames.at(key - d->valueOffset),
            d->valueColumns.at(key - d->valueOffset)->toString(value),
            (d->currentRow() + key)->toString());

    return true;
}

QT_END_NAMESPACE_DOCGALLERY

#include "moc_qgallerytrackereditableresultset_p.cpp"
