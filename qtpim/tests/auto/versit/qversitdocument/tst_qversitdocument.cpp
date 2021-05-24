/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
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

//TESTED_COMPONENT=src/versit

#include "tst_qversitdocument.h"
#include <QtVersit/qversitdocument.h>
#include <QtVersit/qversitproperty.h>
#include <QString>
#include <QtTest/QtTest>

QTVERSIT_USE_NAMESPACE

void tst_QVersitDocument::init()
{
    mVersitDocument = new QVersitDocument();
    QVERIFY(mVersitDocument);
}

void tst_QVersitDocument::cleanup()
{
    delete mVersitDocument;
}


void tst_QVersitDocument::testConstructor()
{
    QCOMPARE(QVersitDocument::InvalidType, mVersitDocument->type());
}

void tst_QVersitDocument::testType()
{
    mVersitDocument->setType(QVersitDocument::VCard21Type);
    QCOMPARE(QVersitDocument::VCard21Type, mVersitDocument->type());

    mVersitDocument->setType(QVersitDocument::VCard30Type);
    QCOMPARE(QVersitDocument::VCard30Type, mVersitDocument->type());
}

void tst_QVersitDocument::testAddProperty()
{
    QCOMPARE(0, mVersitDocument->properties().count());
    QVersitProperty property;
    mVersitDocument->addProperty(property);
    QCOMPARE(1, mVersitDocument->properties().count());
}

void tst_QVersitDocument::testRemoveProperty()
{
    // Remove an empty property.
    QCOMPARE(mVersitDocument->properties().count(), 0);
    QVersitProperty property;
    mVersitDocument->addProperty(property);
    mVersitDocument->removeProperty(property);
    QCOMPARE(mVersitDocument->properties().count(), 0);

    // A full property.
    property.setName(QStringLiteral("TEL"));
    property.setGroups(QStringList(QStringLiteral("HOME")));
    QMultiHash<QString, QString> params;
    params.insert(QStringLiteral("TYPE"), QStringLiteral("HOME"));
    property.setParameters(params);
    property.setValue(QStringLiteral("123"));
    mVersitDocument->addProperty(property);
    QCOMPARE(mVersitDocument->properties().count(), 1);
    QVersitProperty property2;
    property2.setName(QStringLiteral("TEL"));
    // Remove with a partial property fails.
    mVersitDocument->removeProperty(property2);
    QCOMPARE(mVersitDocument->properties().count(), 1);
    property2.setGroups(QStringList(QStringLiteral("HOME")));
    property2.setParameters(params);
    property2.setValue(QStringLiteral("123"));
    // Remove with a fully specified property succeeds.
    mVersitDocument->removeProperty(property2);
    QCOMPARE(mVersitDocument->properties().count(), 0);
}

void tst_QVersitDocument::testRemoveAllProperties()
{
    QString name(QStringLiteral("FN"));

    // Try to remove from an empty document
    QCOMPARE(0, mVersitDocument->properties().count());
    mVersitDocument->removeProperties(name);
    QCOMPARE(0, mVersitDocument->properties().count());

    // Try to remove from a non-empty document, name does not match
    QVersitProperty property;
    property.setName(QStringLiteral("TEL"));
    mVersitDocument->addProperty(property);
    QCOMPARE(1, mVersitDocument->properties().count());
    mVersitDocument->removeProperties(name);
    QCOMPARE(1, mVersitDocument->properties().count());

    // Remove from a non-empty document, name matches
    mVersitDocument->removeProperties(QStringLiteral("TEL"));
    QCOMPARE(0, mVersitDocument->properties().count());

    // Remove from a document with two properties, first matches
    property.setName(name);
    mVersitDocument->addProperty(property);
    property.setName(QStringLiteral("TEL"));
    mVersitDocument->addProperty(property);
    QCOMPARE(2, mVersitDocument->properties().count());
    mVersitDocument->removeProperties(name);
    QCOMPARE(1, mVersitDocument->properties().count());

    // Remove from a document with two properties, second matches
    property.setName(name);
    mVersitDocument->addProperty(property);
    QCOMPARE(2, mVersitDocument->properties().count());
    mVersitDocument->removeProperties(name);
    QCOMPARE(1, mVersitDocument->properties().count());
}

void tst_QVersitDocument::testEquality()
{
    QVersitDocument document1;
    QVersitDocument document2;
    QVERIFY(document1.isEmpty());
    QVERIFY(document1 == document2);
    QVERIFY(!(document1 != document2));
    QVersitProperty property;
    property.setName(QStringLiteral("FN"));
    property.setValue(QStringLiteral("John Citizen"));
    document2.addProperty(property);
    QVERIFY(!(document1 == document2));
    QVERIFY(document1 != document2);
    QVERIFY(!document2.isEmpty());

    document1.addProperty(property);
    QVERIFY(document1 == document2);
    QVERIFY(!(document1 != document2));

    document2.clear();
    QVERIFY(document2.isEmpty());

    document1.clear();
    QVERIFY(document1 == document2);
    QVERIFY(!(document1 != document2));

    document2.setType(QVersitDocument::VCard21Type);
    QVERIFY(!(document1 == document2));
    QVERIFY(document1 != document2);
    QVERIFY(!document2.isEmpty());

    document2 = document1;
    QVERIFY(document1 == document2);
}

void tst_QVersitDocument::testHash()
{
    QVersitDocument document1;
    document1.setType(QVersitDocument::VCard30Type);
    QVersitProperty property1;
    property1.setName(QStringLiteral("name"));
    property1.setValue(QStringLiteral("value"));
    document1.addProperty(property1);

    QVersitDocument document2;
    document2.setType(QVersitDocument::VCard30Type);
    document2.addProperty(property1);

    QVersitDocument document3;
    document3.setType(QVersitDocument::VCard30Type);
    QVersitProperty property3;
    property3.setName(QStringLiteral("name"));
    property3.setValue(QStringLiteral("another value"));
    document3.addProperty(property3);

    QVersitDocument document4; // no properties
    document4.setType(QVersitDocument::VCard30Type);

    QVersitDocument document5 = document1;
    document5.addSubDocument(document4);

    QVersitDocument document6 = document1;
    document6.setComponentType(QStringLiteral("VEVENT"));

    QVersitDocument document7 = document1;
    document7.addProperty(QVersitProperty());

    QVERIFY(qHash(document1) == qHash(document2));
    QVERIFY(qHash(document1) != qHash(document3));
    QVERIFY(qHash(document1) != qHash(document4));
    QVERIFY(qHash(document1) != qHash(document5));
    QVERIFY(qHash(document1) != qHash(document6));
    QVERIFY(qHash(document1) != qHash(document7));
}

QTEST_MAIN(tst_QVersitDocument)

