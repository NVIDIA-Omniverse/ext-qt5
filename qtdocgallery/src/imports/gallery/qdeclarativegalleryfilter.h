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

#ifndef QDECLARATIVEGALLERYFILTER_H
#define QDECLARATIVEGALLERYFILTER_H

#include <qgalleryfilter.h>
#include <QtQml/qqml.h>

QT_BEGIN_NAMESPACE_DOCGALLERY

class QGalleryFilter;

class QDeclarativeGalleryFilterBase : public QObject
{
    Q_OBJECT
public:
    explicit QDeclarativeGalleryFilterBase(QObject *parent = Q_NULLPTR)
        : QObject(parent)
    {
    }

    virtual QGalleryFilter filter() const = 0;

Q_SIGNALS:
    void filterChanged();
};

enum Comparator
{
    Equals,
    LessThan,
    GreaterThan,
    LessThanEquals,
    GreaterThanEquals,
    Contains,
    StartsWith,
    EndsWith,
    Wildcard,
    RegExp
};

class QDeclarativeGalleryValueFilter : public QDeclarativeGalleryFilterBase
{
    Q_OBJECT
    Q_PROPERTY(QString property READ propertyName WRITE setPropertyName NOTIFY propertyNameChanged)
    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(bool negated READ isNegated WRITE setNegated NOTIFY negatedChanged)
public:
    QString propertyName() const { return m_filter.propertyName(); }
    void setPropertyName(const QString &name);

    QVariant value() const { return m_filter.value(); }
    void setValue(const QVariant &value);

    bool isNegated() const { return m_filter.isNegated(); }
    void setNegated(bool negated);

    QGalleryFilter filter() const;

Q_SIGNALS:
    void propertyNameChanged();
    void valueChanged();
    void negatedChanged();

protected:
    explicit QDeclarativeGalleryValueFilter(
            QGalleryFilter::Comparator comparator, QObject *parent = Q_NULLPTR)
        : QDeclarativeGalleryFilterBase(parent)
    {
        m_filter.setComparator(comparator);
    }

    QGalleryMetaDataFilter m_filter;
};

class QDeclarativeGalleryStringFilter : public QDeclarativeGalleryFilterBase
{
    Q_OBJECT
    Q_PROPERTY(QString property READ propertyName WRITE setPropertyName NOTIFY propertyNameChanged)
    Q_PROPERTY(QString value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(bool negated READ isNegated WRITE setNegated NOTIFY negatedChanged)
public:
    QString propertyName() const { return m_filter.propertyName(); }
    void setPropertyName(const QString &name);

    QString value() const { return m_filter.value().toString(); }
    void setValue(const QString &value);

    bool isNegated() const { return m_filter.isNegated(); }
    void setNegated(bool negated);

    QGalleryFilter filter() const;

Q_SIGNALS:
    void propertyNameChanged();
    void valueChanged();
    void negatedChanged();

protected:
    explicit QDeclarativeGalleryStringFilter(
            QGalleryFilter::Comparator comparator, QObject *parent = Q_NULLPTR)
        : QDeclarativeGalleryFilterBase(parent)
    {
        m_filter.setComparator(comparator);
    }

    QGalleryMetaDataFilter m_filter;
};

class QDeclarativeGalleryEqualsFilter : public QDeclarativeGalleryValueFilter
{
    Q_OBJECT
public:
    explicit QDeclarativeGalleryEqualsFilter(QObject *parent = Q_NULLPTR)
        : QDeclarativeGalleryValueFilter(QGalleryFilter::Equals, parent)
    {
    }

    QGalleryFilter filter() const;
};

class QDeclarativeGalleryLessThanFilter : public QDeclarativeGalleryValueFilter
{
    Q_OBJECT
public:
    explicit QDeclarativeGalleryLessThanFilter(QObject *parent = Q_NULLPTR)
        : QDeclarativeGalleryValueFilter(QGalleryFilter::LessThan, parent)
    {
    }
};

class QDeclarativeGalleryLessThanEqualsFilter : public QDeclarativeGalleryValueFilter
{
    Q_OBJECT
public:
    explicit QDeclarativeGalleryLessThanEqualsFilter(QObject *parent = Q_NULLPTR)
        : QDeclarativeGalleryValueFilter(QGalleryFilter::LessThanEquals, parent)
    {
    }
};

class QDeclarativeGalleryGreaterThanFilter : public QDeclarativeGalleryValueFilter
{
    Q_OBJECT
public:
    explicit QDeclarativeGalleryGreaterThanFilter(QObject *parent = Q_NULLPTR)
        : QDeclarativeGalleryValueFilter(QGalleryFilter::GreaterThan, parent)
    {
    }
};

class QDeclarativeGalleryGreaterThanEqualsFilter : public QDeclarativeGalleryValueFilter
{
    Q_OBJECT
public:
    explicit QDeclarativeGalleryGreaterThanEqualsFilter(QObject *parent = Q_NULLPTR)
        : QDeclarativeGalleryValueFilter(QGalleryFilter::GreaterThanEquals, parent)
    {
    }
};

class QDeclarativeGalleryContainsFilter : public QDeclarativeGalleryStringFilter
{
    Q_OBJECT
public:
    explicit QDeclarativeGalleryContainsFilter(QObject *parent = Q_NULLPTR)
        : QDeclarativeGalleryStringFilter(QGalleryFilter::Contains, parent)
    {
    }
};

class QDeclarativeGalleryStartsWithFilter : public QDeclarativeGalleryStringFilter
{
    Q_OBJECT
public:
    explicit QDeclarativeGalleryStartsWithFilter(QObject *parent = Q_NULLPTR)
        : QDeclarativeGalleryStringFilter(QGalleryFilter::StartsWith, parent)
    {
    }
};


class QDeclarativeGalleryEndsWithFilter : public QDeclarativeGalleryStringFilter
{
    Q_OBJECT
public:
    explicit QDeclarativeGalleryEndsWithFilter(QObject *parent = Q_NULLPTR)
        : QDeclarativeGalleryStringFilter(QGalleryFilter::EndsWith, parent)
    {
    }
};

class QDeclarativeGalleryWildcardFilter : public QDeclarativeGalleryStringFilter
{
    Q_OBJECT
public:
    explicit QDeclarativeGalleryWildcardFilter(QObject *parent = Q_NULLPTR)
        : QDeclarativeGalleryStringFilter(QGalleryFilter::Wildcard, parent)
    {
    }
};

class QDeclarativeGalleryFilterGroup
    : public QDeclarativeGalleryFilterBase
    , public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QQmlListProperty<QDocGallery::QDeclarativeGalleryFilterBase> filters READ filters)
    Q_CLASSINFO("DefaultProperty", "filters")
public:
    explicit QDeclarativeGalleryFilterGroup(QObject *parent = Q_NULLPTR)
        : QDeclarativeGalleryFilterBase(parent)
        , m_complete(false)
    {
    }

    void classBegin();
    void componentComplete();

    QQmlListProperty<QDeclarativeGalleryFilterBase> filters();

protected:
    QList<QDeclarativeGalleryFilterBase *> m_filters;

private:
    bool m_complete;

    static void append(
            QQmlListProperty<QDeclarativeGalleryFilterBase> *filters,
            QDeclarativeGalleryFilterBase *filter);
    static int count(QQmlListProperty<QDeclarativeGalleryFilterBase> *filters);
    static QDeclarativeGalleryFilterBase *at(
            QQmlListProperty<QDeclarativeGalleryFilterBase> *filters, int index);
    static void clear(QQmlListProperty<QDeclarativeGalleryFilterBase> *filters);
};

class QDeclarativeGalleryFilterUnion : public QDeclarativeGalleryFilterGroup
{
    Q_OBJECT
public:
    explicit QDeclarativeGalleryFilterUnion(QObject *parent = Q_NULLPTR)
        : QDeclarativeGalleryFilterGroup(parent)
    {
    }

    QGalleryFilter filter() const;
};

class QDeclarativeGalleryFilterIntersection : public QDeclarativeGalleryFilterGroup
{
    Q_OBJECT
public:
    explicit QDeclarativeGalleryFilterIntersection(QObject *parent = Q_NULLPTR)
        : QDeclarativeGalleryFilterGroup(parent)
    {
    }

    QGalleryFilter filter() const;
};

QT_END_NAMESPACE_DOCGALLERY

QML_DECLARE_TYPE(QT_DOCGALLERY_PREPEND_NAMESPACE(QDeclarativeGalleryFilterBase))
QML_DECLARE_TYPE(QT_DOCGALLERY_PREPEND_NAMESPACE(QDeclarativeGalleryEqualsFilter))
QML_DECLARE_TYPE(QT_DOCGALLERY_PREPEND_NAMESPACE(QDeclarativeGalleryLessThanFilter))
QML_DECLARE_TYPE(QT_DOCGALLERY_PREPEND_NAMESPACE(QDeclarativeGalleryLessThanEqualsFilter))
QML_DECLARE_TYPE(QT_DOCGALLERY_PREPEND_NAMESPACE(QDeclarativeGalleryGreaterThanFilter))
QML_DECLARE_TYPE(QT_DOCGALLERY_PREPEND_NAMESPACE(QDeclarativeGalleryGreaterThanEqualsFilter))
QML_DECLARE_TYPE(QT_DOCGALLERY_PREPEND_NAMESPACE(QDeclarativeGalleryContainsFilter))
QML_DECLARE_TYPE(QT_DOCGALLERY_PREPEND_NAMESPACE(QDeclarativeGalleryStartsWithFilter))
QML_DECLARE_TYPE(QT_DOCGALLERY_PREPEND_NAMESPACE(QDeclarativeGalleryEndsWithFilter))
QML_DECLARE_TYPE(QT_DOCGALLERY_PREPEND_NAMESPACE(QDeclarativeGalleryWildcardFilter))
QML_DECLARE_TYPE(QT_DOCGALLERY_PREPEND_NAMESPACE(QDeclarativeGalleryFilterUnion))
QML_DECLARE_TYPE(QT_DOCGALLERY_PREPEND_NAMESPACE(QDeclarativeGalleryFilterIntersection))

#endif
