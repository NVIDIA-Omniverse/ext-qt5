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

#include "qscreensaver.h"

#if defined(Q_OS_LINUX)
#  if defined(QT_UNITY8)
#    include "linux/qscreensaver_mir_p.h"
#  else
#    include "linux/qscreensaver_linux_p.h"
#  endif
#elif defined(Q_OS_WIN)
#  include "windows/qscreensaver_win_p.h"
#elif defined(Q_OS_MAC)
#  include "mac/qscreensaver_mac_p.h"
#else
QT_BEGIN_NAMESPACE
class QScreenSaverPrivate
{
public:
    QScreenSaverPrivate(QScreenSaver *) {}

    bool screenSaverEnabled() { return false; }
    void setScreenSaverEnabled(bool) {}
};
QT_END_NAMESPACE
#endif

QT_BEGIN_NAMESPACE

/*!
    \class QScreenSaver
    \inmodule QtSystemInfo
    \brief The QScreenSaver class provides various information about the screen saver.

    \ingroup systeminfo
*/

/*!
    Constructs a QScreenSaver object with the given \a parent.
*/
QScreenSaver::QScreenSaver(QObject *parent)
    : QObject(parent)
    , d_ptr(new QScreenSaverPrivate(this))
{
}

/*!
    Destroys the object
*/
QScreenSaver::~QScreenSaver()
{
    delete d_ptr;
}

/*!
    \property QScreenSaver::screenSaverEnabled
    \brief The state of the screen saver.

    Returns if the screen saver is enabled.
*/
bool QScreenSaver::screenSaverEnabled() const
{
    return d_ptr->screenSaverEnabled();
}

/*!
    Sets the screen saver to be \a enabled.
*/
void QScreenSaver::setScreenSaverEnabled(bool enabled)
{
    d_ptr->setScreenSaverEnabled(enabled);
}

QT_END_NAMESPACE
