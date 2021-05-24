/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the documentation of the Qt PIM Module.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

//! [Basic list Complete Snippet]
import QtQuick 2.0
import QtOrganizer 5.0

Rectangle {
    id : organizerSample
    width: 200
    height: 400

    Rectangle {
        id:organizerEventView

        anchors.fill: parent
        color: "white"

//! [Organizer Model]
        OrganizerModel {
            id: organizer

            startPeriod: "1970-01-01"
            endPeriod:'2012-12-31'
            autoUpdate:true

            //! [Manager Choice]
            manager:"qtorganizer:memory:id=qml"
            //! [Manager Choice]

            Component.onCompleted : {
                if (managerName == "memory") {
                    organizer.importItems(Qt.resolvedUrl("content/organizer_ical_test.ics"));
                    console.log("Memory backend : import Items executed.");
                }
            }
        }
//! [Organizer Model]

//! [List View]
        ListView {
            id: calendar
            anchors.fill: parent

            clip: true

            delegate: Text { text: model.item.displayLabel }
            model: organizer
        }
//! [List View]
    }
}
//! [Basic list Complete Snippet]
