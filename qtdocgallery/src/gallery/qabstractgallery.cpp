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

#include "qabstractgallery.h"
#include "qabstractgallery_p.h"

QT_BEGIN_NAMESPACE_DOCGALLERY

/*!
    \class QAbstractGallery

    \ingroup gallery
    \ingroup gallery-galleries

    \inmodule QtDocGallery

    \brief The QAbstractGallery class provides a base class for gallery
    implementations.

    Classes implementing the QAbstractGallery interface allow
    QGalleryAbstractRequest implementations to connect to a gallery and
    execute a query or perform an action.

    The types of request that can be executed by a gallery are defined in
    QGalleryAbstractRequest::RequestType.  If a gallery supports a request the
    isRequestSupportedFunction() will return true when given the type of the
    request, and false if it is not supported.

    When a QGalleryAbstractRequest is executed it will call createResponse() on
    its gallery and pass a pointer to itself.  The gallery will then use the
    \l {QGalleryAbstractRequest::type()}{type} property of the request to
    determine its type and cast to the correct class.  Using the properties of
    the request a gallery can then create a QGalleryAbstractResponse which
    executes the requested task and returns any results.  Some requests will
    require the response to implement additional interfaces such as
    QGalleryResultSet to access the data produced in the response.

    \sa QGalleryAbstractRequest
*/

/*!
    Constructs a new gallery.

    The \a parent is passed to QObject.
*/

QAbstractGallery::QAbstractGallery(QObject *parent )
    : QObject(parent)
    , d_ptr(0)
{

}

/*!
    \internal
*/

QAbstractGallery::QAbstractGallery(QAbstractGalleryPrivate &dd, QObject *parent)
    : QObject(parent)
    , d_ptr(&dd)
{
    d_ptr->q_ptr = this;
}

/*!
    Destroys a gallery.
*/

QAbstractGallery::~QAbstractGallery()
{
}

/*!
    \fn QAbstractGallery::isRequestSupported(QGalleryAbstractRequest::RequestType type) const

    Identifies if a gallery supports a request \a type.

    Returns true if the request is supported, and false otherwise.
*/

/*!
    \fn QAbstractGallery::createResponse(QGalleryAbstractRequest *request)

    Creates a response to a gallery \a request.
*/

#include "moc_qabstractgallery.cpp"

QT_END_NAMESPACE_DOCGALLERY
