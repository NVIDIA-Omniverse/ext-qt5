/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef OBJECT_ENDPOINT_DBUS_H
#define OBJECT_ENDPOINT_DBUS_H

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

#include "qserviceframeworkglobal.h"
#include "ipcendpoint_p.h"
#include "qremoteserviceregister.h"
#include "qservice.h"
#include <QPointer>
#include <QHash>
#include <QtDBus>
#include "qservicemetaobject_dbus_p.h"

QT_BEGIN_NAMESPACE

class QServiceMetaObjectDBus;
class ObjectEndPointPrivate;
class ObjectEndPoint : public QObject
{
    Q_OBJECT
public:
    enum Type {
        Service = 0,
        Client
    };

    ObjectEndPoint(Type type, QServiceIpcEndPoint* comm, QObject* parent = 0);
    ~ObjectEndPoint();

    QObject* constructProxy(const QRemoteServiceRegister::Entry& entry);

    void objectRequest(const QServicePackage& p, QServiceClientCredentials &creds);
    void methodCall(const QServicePackage& p);
    void propertyCall(const QServicePackage& p);

    QString getInstanceId() const;

    QVariant invokeRemote(int metaIndex, const QVariantList& args, int returnType);
    QVariant invokeRemoteProperty(int metaIndex, const QVariant& arg, int returnType, QMetaObject::Call c);

    void setLookupTable(int *local, int *remote);

Q_SIGNALS:
    void pendingRequestFinished();

public Q_SLOTS:
    void newPackageReady();
    void disconnected(const QString& clientId, const QString & instanceId);
    void unregisterObjectDBus(const QRemoteServiceRegister::Entry& entry, const QUuid& id);

private:
    void waitForResponse(const QUuid& requestId);
    QVariant toDBusVariant(const QByteArray& type, const QVariant& arg);

    QServiceIpcEndPoint* dispatch;
    QPointer<QObject> service;
    ObjectEndPointPrivate* d;

    QDBusInterface *iface;
    QServiceMetaObjectDBus *signalsObject;
    int *localToRemote;
    int *remoteToLocal;

};

QT_END_NAMESPACE

#endif //OBJECT_ENDPOINT_DBUS_H
