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

//TESTED_COMPONENT=src/contacts

#include <QtTest/QtTest>
#include <QApplication>

#include <QtContacts/qtcontacts.h>
#include <QtContacts/qcontactaction.h>
#include <QtContacts/qcontactactiondescriptor.h>

#include "qservicemanager.h"

QTCONTACTS_USE_NAMESPACE

class tst_QContactActions : public QObject
{
Q_OBJECT

public:
    tst_QContactActions();
    virtual ~tst_QContactActions();

public slots:
    void init();
    void cleanup();
private slots:
    void contactFunctions();
    void testSendEmail();
    void testDescriptor();
    void testDescriptorHash();
    void testTarget();
    void traits();
};

tst_QContactActions::tst_QContactActions()
{
    // set the correct path to look for plugins
    QString path = QApplication::applicationDirPath() + "/dummyplugin/plugins";
    QApplication::addLibraryPath(path);

    // and add the sendemail + call actions to the service framework
    QServiceManager sm;

    // clean up any actions/services.
    QStringList allServices = sm.findServices();
    foreach(const QString& serv, allServices) {
        if (serv.startsWith("tst_qcontact")) {
            if (!sm.removeService(serv)) {
                qDebug() << " tst_qca: ctor: cleaning up test service" << serv << "failed:" << sm.error();
            }
        }
    }

    if (!sm.addService(QCoreApplication::applicationDirPath() + "/plugins/contacts/xmldata/sendemailactionservice.xml"))
        qDebug() << " tst_qca: ctor: unable to add SendEmail service:" << sm.error();
    if (!sm.addService(QCoreApplication::applicationDirPath() + "/plugins/contacts/xmldata/multiactionservice.xml"))
        qDebug() << " tst_qca: ctor: unable to add MultiAction service:" << sm.error();
}

tst_QContactActions::~tst_QContactActions()
{
    QString path = QApplication::applicationDirPath() + "/dummyplugin/plugins";
    QApplication::removeLibraryPath(path);

    // clean up any actions/services.
    QServiceManager sm;
    QStringList allServices = sm.findServices();
    foreach(const QString& serv, allServices) {
        if (serv.startsWith("tst_qcontact")) {
            if (!sm.removeService(serv)) {
                qDebug() << " tst_qca: ctor: cleaning up test service" << serv << "failed:" << sm.error();
            }
        }
    }
}

void tst_QContactActions::init()
{
}

void tst_QContactActions::cleanup()
{
}

void tst_QContactActions::contactFunctions()
{
    QContact c;  // empty contact.
    QContact c2; // contact with email saved.
    QContact c3; // two emails
    QContact c4; // two emails, plus a preference
    QContact c5; // two emails, plus a preference for an unsupported detail
    QContactEmailAddress e2;
    QContactEmailAddress e;
    e.setEmailAddress("test@nokia.com");
    c2.saveDetail(&e);
    e2.setEmailAddress("secondtest@nokia.com");

    c3.saveDetail(&e);
    c3.saveDetail(&e2);
    c4.saveDetail(&e);
    c4.saveDetail(&e2);
    c5.saveDetail(&e2); // reverse order for c5
    c5.saveDetail(&e);

    c4.setPreferredDetail("SendEmail", e2);
    c5.setPreferredDetail("SendEmail", c5.detail<QContactDisplayLabel>());

    // Get our descriptor
    QContactActionDescriptor descriptor = QContactAction::actionDescriptors("SendEmail").value(0);
    QVERIFY(descriptor.actionName() == "SendEmail");

    // available actions:
    // empty contact
    QList<QContactActionDescriptor> availableActions = c.availableActions(QString());
    QVERIFY(availableActions.isEmpty());  // should not contain SendEmail
    // contact with email
    availableActions = c2.availableActions(QString());
    QVERIFY(!availableActions.isEmpty()); // should contain SendEmail
    QVERIFY(availableActions.contains(descriptor));

    // try various combinations of version and name
    availableActions = c2.availableActions();
    QVERIFY(!availableActions.isEmpty()); // should contain SendEmail
    QVERIFY(availableActions.contains(descriptor));
    availableActions = c2.availableActions("tst_qcontactactions:sendemailaction");
    QVERIFY(!availableActions.isEmpty()); // should contain SendEmail
    availableActions = c2.availableActions(QString());
    QVERIFY(!availableActions.isEmpty()); // should contain SendEmail
    QVERIFY(availableActions.contains(descriptor));

    // Again with c3
    availableActions = c3.availableActions(QString());
    QVERIFY(!availableActions.isEmpty()); // should contain SendEmail
    QVERIFY(availableActions.contains(descriptor));

    // try various combinations of version and name
    availableActions = c3.availableActions();
    QVERIFY(!availableActions.isEmpty()); // should contain SendEmail
    QVERIFY(availableActions.contains(descriptor));
    availableActions = c3.availableActions("tst_qcontactactions:sendemailaction");
    QVERIFY(!availableActions.isEmpty()); // should contain SendEmail
    QVERIFY(availableActions.contains(descriptor));
    availableActions = c3.availableActions(QString());
    QVERIFY(!availableActions.isEmpty()); // should contain SendEmail
    QVERIFY(availableActions.contains(descriptor));
}

void tst_QContactActions::testSendEmail()
{
    QContact c;
    QContactEmailAddress e;
    e.setEmailAddress("test@nokia.com");
    c.saveDetail(&e);

    QVERIFY(QContactAction::availableActions().contains("SendEmail"));
    QVERIFY(QContactAction::availableActions("tst_qcontactactions:sendemailaction").contains("SendEmail"));

    QList<QContactActionDescriptor> descrs = QContactAction::actionDescriptors();
    bool foundSendEmail = false;
    for (int i = 0; i < descrs.size(); i++) {
        if (descrs.at(i).actionName() == QString("SendEmail")) {
            foundSendEmail = true;
            break;
        }
    }
    QVERIFY(foundSendEmail);

    descrs = QContactAction::actionDescriptors(QString());
    foundSendEmail = false;
    for (int i = 0; i < descrs.size(); i++) {
        QCOMPARE(descrs.at(i).serviceName(), QString("tst_qcontactactions:sendemailaction"));
        if (descrs.at(i).actionName() == QString("SendEmail")) {
            foundSendEmail = true;
            break;
        }
    }
    QVERIFY(foundSendEmail);

    descrs = QContactAction::actionDescriptors(QString());
    foundSendEmail = false;
    for (int i = 0; i < descrs.size(); i++) {
        QCOMPARE(descrs.at(i).serviceName(), QString("tst_qcontactactions:sendemailaction"));
        QCOMPARE(descrs.at(i).implementationVersion(), 1);
        if (descrs.at(i).actionName() == QString("SendEmail")
                && descrs.at(i).serviceName() == QString("tst_qcontactactions:sendemailaction")
                && descrs.at(i).implementationVersion() == 1) {
            foundSendEmail = true;
            break;
        }
    }
    QVERIFY(foundSendEmail);

    descrs = QContactAction::actionDescriptors("SendEmail");
    foundSendEmail = false;
    for (int i = 0; i < descrs.size(); i++) {
        QCOMPARE(descrs.at(i).actionName(), QString("SendEmail"));
        if (descrs.at(i).actionName() == QString("SendEmail")
                && descrs.at(i).serviceName() == QString("tst_qcontactactions:sendemailaction")
                && descrs.at(i).implementationVersion() == 1) {
            foundSendEmail = true;
            break;
        }
    }
    QVERIFY(foundSendEmail);

    descrs = QContactAction::actionDescriptors();
    QContactAction* sendEmail = 0;
    for (int i = 0; i < descrs.size(); i++) {
        if (descrs.at(i).actionName() == QString("SendEmail")
                && descrs.at(i).serviceName() == QString("tst_qcontactactions:sendemailaction")
                && descrs.at(i).implementationVersion() == 1) {
            sendEmail = QContactAction::action(descrs.at(i));
            break;
        }
    }
    QVERIFY(sendEmail);
}

void tst_QContactActions::testDescriptor()
{
    // first, test retrieving an invalid action
    QContactAction* invalidAction = QContactAction::action(QContactActionDescriptor());
    QVERIFY(invalidAction == 0); // should be null.

    QContact c;
    QContactEmailAddress e;
    e.setEmailAddress("test@nokia.com");
    c.saveDetail(&e);
    QContactPhoneNumber p;
    p.setNumber("12345");
    c.saveDetail(&p);

    QVERIFY(QContactAction::availableActions().contains("SendEmail"));
    QVERIFY(QContactAction::availableActions("tst_qcontactactions:sendemailaction").contains("SendEmail"));

    QContactActionDescriptor sendEmailDescriptor;
    QContactActionDescriptor multiActionOneDescriptor;
    QContactActionDescriptor multiActionTwoDescriptor;

    QList<QContactActionDescriptor> descrs = QContactAction::actionDescriptors();
    QContactAction* sendEmailAction = 0;
    for (int i = 0; i < descrs.size(); i++) {
        QContactActionDescriptor temp = descrs.at(i);
        if (temp.actionName() == QString("SendEmail")) {
            sendEmailAction = QContactAction::action(temp);
            QVERIFY(c.availableActions().contains(temp)); // has an email address, so should be available
            QVERIFY(temp.supportsContact(c));
            sendEmailDescriptor = temp;
        } else if (temp.actionName() == QString("call")) {
            if (temp.metaData(QString(QLatin1String("Provider"))) == QString(QLatin1String("sip"))) {
                multiActionOneDescriptor = temp;
            } else {
                multiActionTwoDescriptor = temp;
            }
        }
    }

    QVERIFY(sendEmailDescriptor.isValid());
    QVERIFY(multiActionOneDescriptor.isValid());
    QVERIFY(multiActionTwoDescriptor.isValid());

    QVERIFY(sendEmailAction != 0);
    delete sendEmailAction;

    // now test equivalence.  The send email action descriptor should
    // have a different action name to both multi action one and two.
    QVERIFY(sendEmailDescriptor.actionName() != multiActionOneDescriptor.actionName());
    QVERIFY(sendEmailDescriptor.actionName() != multiActionTwoDescriptor.actionName());
    QVERIFY(sendEmailDescriptor != multiActionOneDescriptor);
    QVERIFY(sendEmailDescriptor != multiActionTwoDescriptor);

    // multi action one and two should have the same action name, service
    // name and implementation (minor) version.  BUT they have different
    // implementations (Provider is different) so they should NOT be equal.
    QVERIFY(multiActionOneDescriptor.actionName() == multiActionTwoDescriptor.actionName());
    QVERIFY(multiActionOneDescriptor.serviceName() == multiActionTwoDescriptor.serviceName());
    QVERIFY(multiActionOneDescriptor.implementationVersion() == multiActionTwoDescriptor.implementationVersion());
    QVERIFY(multiActionOneDescriptor != multiActionTwoDescriptor);

    // verify that the meta data is reported correctly
    QVERIFY(multiActionOneDescriptor.metaData("Provider") != multiActionTwoDescriptor.metaData("Provider"));
}

void tst_QContactActions::testDescriptorHash()
{
    QContactActionDescriptor qcad1, qcad2, qcad3;
    QList<QContactActionDescriptor> alldes = QContactAction::actionDescriptors();

    if (alldes.size() > 1) {
        qcad1 = alldes.at(0);
        qcad2 = alldes.at(1);
        QVERIFY(qHash(qcad1) != qHash(qcad2));
        qcad3 = alldes.at(0);
        QVERIFY(qHash(qcad1) == qHash(qcad3));
    }

    if (alldes.size() > 2) {
        qcad3 = alldes.at(2);
        QVERIFY(qHash(qcad1) != qHash(qcad2));
        QVERIFY(qHash(qcad2) != qHash(qcad3));
        QVERIFY(qHash(qcad1) != qHash(qcad3));
    }
}

void tst_QContactActions::testTarget()
{
    // first, create a contact with some details
    QContact c1;
    QContactName n1;
    n1.setFirstName("test");
    n1.setLastName("contact");
    c1.saveDetail(&n1);
    QContactPhoneNumber p1;
    p1.setNumber("12345");
    c1.saveDetail(&p1);
    QContactEmailAddress e1;
    e1.setEmailAddress("test@example.com");
    c1.saveDetail(&e1);

    QList<QContactDetail> dl1;
    dl1 << p1 << e1;

    QContactActionTarget t1;          // default ctor
    QContactActionTarget t2(c1);      // "whole contact" target
    QContactActionTarget t3(c1, e1);  // "specific detail" target
    QContactActionTarget t4(c1, dl1); // "detail list" target

    QCOMPARE(t2.contact(), c1);
    QCOMPARE(t3.contact(), c1);
    QCOMPARE(t4.contact(), c1);

    QCOMPARE(t2.details(), QList<QContactDetail>());
    QCOMPARE(t3.details(), QList<QContactDetail>() << e1);
    QCOMPARE(t4.details(), dl1);

    QVERIFY(t1 != t2);
    QVERIFY(t1 != t3);
    QVERIFY(t1 != t4);
    QVERIFY(t2 != t3);
    QVERIFY(t2 != t4);
    QVERIFY(t3 != t4);
    QVERIFY(qHash(t2) != qHash(t3));
    QVERIFY(qHash(t2) != qHash(t4));
    QVERIFY(qHash(t3) != qHash(t4));

    QVERIFY(!t1.isValid());
    QVERIFY(t2.isValid());
    QVERIFY(t3.isValid());
    QVERIFY(t4.isValid());

    t1.setContact(c1);
    t1.setDetails(dl1);
    QVERIFY(t1.isValid());
    QVERIFY(t1 == t4);

    t2 = t1;
    QVERIFY(t2.isValid()); // check that assignment operator doesn't destroy validity.
    QVERIFY(t1 == t2);
    QVERIFY(t2 == t4);
    QVERIFY(t2 != t3);
}

void tst_QContactActions::traits()
{
    QCOMPARE(sizeof(QContactActionDescriptor), sizeof(void *));
    QTypeInfo<QTM_PREPEND_NAMESPACE(QContactActionDescriptor)> ti;
    QVERIFY(ti.isComplex);
    QVERIFY(!ti.isStatic);
    QVERIFY(!ti.isLarge);
    QVERIFY(!ti.isPointer);
    QVERIFY(!ti.isDummy);
}

QTEST_MAIN(tst_QContactActions)
#include "tst_qcontactactions.moc"
