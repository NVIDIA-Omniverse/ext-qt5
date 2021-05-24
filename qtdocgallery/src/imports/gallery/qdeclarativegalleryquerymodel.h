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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef GALLERYQUERYREQUEST_H
#define GALLERYQUERYREQUEST_H

#include <qgalleryqueryrequest.h>

#include "qdeclarativedocumentgallery.h"
#include "qdeclarativegalleryfilter.h"

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qpointer.h>
#include <QtQml/qqml.h>
#include <QtQml/qjsvalue.h>

QT_BEGIN_NAMESPACE_DOCGALLERY

class QDeclarativeGalleryFilterBase;

class QDeclarativeGalleryQueryModel : public QAbstractListModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_ENUMS(Status)
    Q_ENUMS(Scope)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(qreal progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QStringList properties READ propertyNames WRITE setPropertyNames NOTIFY propertyNamesChanged)
    Q_PROPERTY(QStringList sortProperties READ sortPropertyNames WRITE setSortPropertyNames NOTIFY sortPropertyNamesChanged)
    Q_PROPERTY(bool autoUpdate READ autoUpdate WRITE setAutoUpdate NOTIFY autoUpdateChanged)
    Q_PROPERTY(QVariant rootItem READ rootItem WRITE setRootItem NOTIFY rootItemChanged)
    Q_PROPERTY(Scope scope READ scope WRITE setScope NOTIFY scopeChanged)
    Q_PROPERTY(int offset READ offset WRITE setOffset NOTIFY offsetChanged)
    Q_PROPERTY(int limit READ limit WRITE setLimit NOTIFY limitChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QDocGallery::QDeclarativeGalleryFilterBase* filter READ filter WRITE setFilter NOTIFY filterChanged)
public:
    enum Status
    {
        Null        = QGalleryAbstractRequest::Inactive,
        Active      = QGalleryAbstractRequest::Active,
        Canceling   = QGalleryAbstractRequest::Canceling,
        Canceled    = QGalleryAbstractRequest::Canceled,
        Idle        = QGalleryAbstractRequest::Idle,
        Finished    = QGalleryAbstractRequest::Finished,
        Error       = QGalleryAbstractRequest::Error
    };

    enum Scope
    {
        AllDescendants = QGalleryQueryRequest::AllDescendants,
        DirectDescendants = QGalleryQueryRequest::DirectDescendants
    };

    enum Roles
    {
        ItemId = 0,
        ItemType,
        MetaDataOffset
    };

    ~QDeclarativeGalleryQueryModel();

    Status status() const { return m_status; }

    qreal progress() const;

    QStringList propertyNames() { return m_request.propertyNames(); }
    void setPropertyNames(const QStringList &names);

    QStringList sortPropertyNames() const { return m_request.sortPropertyNames(); }
    void setSortPropertyNames(const QStringList &names);

    bool autoUpdate() const { return m_request.autoUpdate(); }
    void setAutoUpdate(bool enabled);

    Scope scope() const { return Scope(m_request.scope()); }
    void setScope(Scope scope);

    QVariant rootItem() const { return m_request.rootItem(); }
    void setRootItem(const QVariant &itemId);

    QDeclarativeGalleryFilterBase *filter() const { return m_filter.data(); }
    void setFilter(QDeclarativeGalleryFilterBase *filter);

    int offset() const { return m_request.offset(); }
    void setOffset(int offset);

    int limit() const { return m_request.limit(); }
    void setLimit(int limit);

    int rowCount(const QModelIndex &parent) const;

    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);

    QModelIndex index(int row, int column, const QModelIndex &parent) const;

    int count() const { return m_rowCount; }

    Q_INVOKABLE QJSValue get(const QJSValue &index) const;
    Q_INVOKABLE QVariant property(int index, const QString &property) const;

    Q_INVOKABLE void set(int index, const QJSValue &value);
    Q_INVOKABLE void setProperty(int index, const QString &property, const QVariant &value);

    void componentComplete();

public Q_SLOTS:
    void reload();
    void cancel();
    void clear();

Q_SIGNALS:
    void statusChanged();
    void progressChanged();
    void propertyNamesChanged();
    void sortPropertyNamesChanged();
    void autoUpdateChanged();
    void rootItemChanged();
    void scopeChanged();
    void filterChanged();
    void offsetChanged();
    void limitChanged();
    void countChanged();

protected Q_SLOTS:
    void deferredExecute();

protected:
    enum UpdateStatus
    {
        Incomplete,
        NoUpdate,
        PendingUpdate,
        CanceledUpdate
    };

    explicit QDeclarativeGalleryQueryModel(QObject *parent = Q_NULLPTR);

    virtual QVariant itemType(const QString &type) const = 0;

    bool event(QEvent *event);

    QGalleryQueryRequest m_request;
    QPointer<QDeclarativeGalleryFilterBase> m_filter;
    QGalleryResultSet *m_resultSet;
    QVector<QPair<int, QString> > m_propertyNames;
    Status m_status;
    int m_rowCount;
    UpdateStatus m_updateStatus;

private Q_SLOTS:
    void _q_stateChanged();
    void _q_setResultSet(QGalleryResultSet *resultSet);
    void _q_itemsInserted(int index, int count);
    void _q_itemsRemoved(int index, int count);
    void _q_itemsMoved(int from, int to, int count);
    void _q_itemsChanged(int index, int count);
};

class QDeclarativeDocumentGalleryModel : public QDeclarativeGalleryQueryModel
{
    Q_OBJECT
    Q_PROPERTY(QDocGallery::QDeclarativeDocumentGallery::ItemType rootType READ rootType WRITE setRootType NOTIFY rootTypeChanged)
public:
    explicit QDeclarativeDocumentGalleryModel(QObject *parent = Q_NULLPTR);
    ~QDeclarativeDocumentGalleryModel();

    void classBegin();

    QDeclarativeDocumentGallery::ItemType rootType() const;
    void setRootType(QDeclarativeDocumentGallery::ItemType itemType);

Q_SIGNALS:
    void rootTypeChanged();

protected:
    QVariant itemType(const QString &type) const;
};

QT_END_NAMESPACE_DOCGALLERY

QML_DECLARE_TYPE(QT_DOCGALLERY_PREPEND_NAMESPACE(QDeclarativeDocumentGalleryModel))

#endif

