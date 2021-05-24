/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtOrganizer module of the Qt Toolkit.
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

#include "qorganizeritemfilter.h"
#include "qorganizeritemfilter_p.h"

#ifndef QT_NO_DATASTREAM
#include <QtCore/qdatastream.h>
#endif
#ifndef QT_NO_DEBUG_STREAM
#include <QtCore/qdebug.h>
#endif

#include "qorganizeritemfilters.h"

/*!
    \class QOrganizerItemFilter
    \brief The QOrganizerItemFilter class is used to filter items made available through a backend.
    \inmodule QtOrganizer
    \ingroup organizer-main

    This class is used as a parameter to various functions offered by QOrganizerManager and QOrganizerAbstractRequest,
    to allow filtering of items which have certain details or properties.
 */

/*!
    \enum QOrganizerItemFilter::FilterType

    This enumeration describes the type of the filter.
    \value InvalidFilter       An invalid filter which matches nothing.
    \value DetailFilter        A filter which matches items containing a detail identical to that
                               used by the filter for matching.
    \value DetailFieldFilter   A filter which matches items containing a detail of a particular type,
                               having a given value for a particular field.
    \value DetailRangeFilter   A filter which matches items containing a detail of a particular type,
                               whose values are within a particular range.
    \value IntersectionFilter  A filter which matches all items that are matched by all filters it includes.
    \value UnionFilter         A filter which matches any organizer item that is matched by any of the
                               filters it includes.
    \value IdFilter            A filter which matches any organizer item whose ID is contained in a
                               particular list of organizer item IDs.
    \value CollectionFilter    A filter which matches any items whose collection ID is contained in
                               a particular list of collection IDs.
    \value DefaultFilter       A filter which matches everything.
 */

/*!
    \enum QOrganizerItemFilter::MatchFlag

    This enumeration describes the semantics of matching followed by the filter.
    \value MatchExactly        Performs QVariant-based matching.
    \value MatchContains       The search term is contained in the item.
    \value MatchStartsWith     The search term matches the start of the item.
    \value MatchEndsWith       The search term matches the end of the item.
    \value MatchFixedString    Performs string-based matching. String-based comparisons are case-insensitive
                               unless the \c MatchCaseSensitive flag is also specified.
    \value MatchCaseSensitive  The search is case sensitive.
 */

/*!
    \fn QOrganizerItemFilter::operator!=(const QOrganizerItemFilter &other) const

    Returns true if this filter is not identical to the \a other filter.

    \sa operator==()
 */

#if !defined(Q_CC_MWERKS)
template<> QTORGANIZER_PREPEND_NAMESPACE(QOrganizerItemFilterPrivate) *QSharedDataPointer<QTORGANIZER_PREPEND_NAMESPACE(QOrganizerItemFilterPrivate)>::clone()
{
    return d->clone();
}
#endif // Q_CC_MWERKS

QT_BEGIN_NAMESPACE_ORGANIZER

/*!
    Constructs an empty filter of type DefaultFilter.
 */
QOrganizerItemFilter::QOrganizerItemFilter()
    : d_ptr(0)
{
}

/*!
    Constructs a new copy of \a other.
 */
QOrganizerItemFilter::QOrganizerItemFilter(const QOrganizerItemFilter &other)
    : d_ptr(other.d_ptr)
{
}

/*!
    Assigns this filter to be \a other.
 */
QOrganizerItemFilter &QOrganizerItemFilter::operator=(const QOrganizerItemFilter &other)
{
    d_ptr = other.d_ptr;
    return *this;
}

/*!
    Cleans up the memory used by this filter.
 */
QOrganizerItemFilter::~QOrganizerItemFilter()
{
}

/*!
    Returns the type of the filter.
 */
QOrganizerItemFilter::FilterType QOrganizerItemFilter::type() const
{
    if (!d_ptr)
        return QOrganizerItemFilter::DefaultFilter;
    return d_ptr->type();
}

/*!
    Returns true if this filter is identical to the \a other filter.

    \sa operator!=()
 */
bool QOrganizerItemFilter::operator==(const QOrganizerItemFilter &other) const
{
    if (d_ptr == other.d_ptr)
        return true;

    if (d_ptr && other.d_ptr)
        return d_ptr->type() == other.d_ptr->type() && d_ptr->compare(other.d_ptr);

    return false;
}

#ifndef QT_NO_DATASTREAM
/*!
    \relates QOrganizerItemFilter
    Writes \a filter to the stream \a out.

    A QOrganizerItemIdFilter will not be preserved if streamed to a QDataStream.
 */
QDataStream &operator<<(QDataStream &out, const QOrganizerItemFilter &filter)
{
    quint8 formatVersion = 1;
    out << formatVersion << static_cast<quint32>(filter.type());
    if (filter.d_ptr)
        filter.d_ptr->outputToStream(out, formatVersion);
    return out;
}

/*!
    \relates QOrganizerItemFilter
    Reads an organizer item filter from stream \a in into \a filter.

    A QOrganizerItemIdFilter will not be preserved if streamed from a QDataStream.
 */
QDataStream &operator>>(QDataStream &in, QOrganizerItemFilter &filter)
{
    quint8 formatVersion;
    in >> formatVersion;
    if (formatVersion == 1) {
        quint32 type;
        in >> type;
        switch (type) {
            case QOrganizerItemFilter::InvalidFilter:
                filter = QOrganizerItemInvalidFilter();
                break;
            case QOrganizerItemFilter::DetailFilter:
                filter = QOrganizerItemDetailFilter();
            case QOrganizerItemFilter::DetailFieldFilter:
                filter = QOrganizerItemDetailFieldFilter();
                break;
            case QOrganizerItemFilter::DetailRangeFilter:
                filter = QOrganizerItemDetailRangeFilter();
                break;
            case QOrganizerItemFilter::IntersectionFilter:
                filter = QOrganizerItemIntersectionFilter();
                break;
            case QOrganizerItemFilter::UnionFilter:
                filter = QOrganizerItemUnionFilter();
                break;
            case QOrganizerItemFilter::IdFilter:
                filter = QOrganizerItemIdFilter();
                break;
            case QOrganizerItemFilter::DefaultFilter:
                filter = QOrganizerItemFilter();
                break;
        }

        if (filter.d_ptr)
            filter.d_ptr->inputFromStream(in, formatVersion);
    } else {
        in.setStatus(QDataStream::ReadCorruptData);
    }
    return in;
}
#endif // QT_NO_DATASTREAM

#ifndef QT_NO_DEBUG_STREAM
/*!
    \relates QOrganizerItemFilter
    Outputs \a filter to the debug stream \a dbg.
 */
QDebug operator<<(QDebug dbg, const QOrganizerItemFilter &filter)
{
    dbg.nospace() << "QOrganizerItemFilter(";
    if (filter.d_ptr)
        filter.d_ptr->debugStreamOut(dbg);
    else
        dbg.nospace() << "(null)";
    dbg.nospace() << ")";
    return dbg.maybeSpace();
}
#endif // QT_NO_DEBUG_STREAM

/*!
    \internal

    Constructs a new filter from the given data pointer \a d.
 */
QOrganizerItemFilter::QOrganizerItemFilter(QOrganizerItemFilterPrivate *d)
    : d_ptr(d)
{
}

/*!
    \relates QOrganizerItemFilter
    Returns a filter which is the intersection of the \a left and \a right filters.

    \sa QOrganizerItemIntersectionFilter
 */
const QOrganizerItemFilter operator&(const QOrganizerItemFilter &left, const QOrganizerItemFilter &right)
{
    // XXX TODO: empty intersection/union operations are not well defined yet.
    //if (left.type() == QOrganizerItemFilter::Intersection) {
    //    QOrganizerItemIntersectionFilter bf(left);
    //    /* we can just add the right to this one */
    //    bf.append(right);
    //    return bf;
    //}

    //if (right.type() == QOrganizerItemFilter::Intersection) {
    //    QOrganizerItemIntersectionFilter bf(right);
    //    /* we can prepend the left to this one */
    //    bf.prepend(left);
    //    return bf;
    //}

    /* usual fallback case */
    QOrganizerItemIntersectionFilter nif;
    nif << left << right;
    return nif;
}

/*!
    \relates QOrganizerItemFilter
    Returns a filter which is the union of the \a left and \a right filters.

    \sa QOrganizerItemUnionFilter
 */
const QOrganizerItemFilter operator|(const QOrganizerItemFilter &left, const QOrganizerItemFilter &right)
{
    if (left.type() == QOrganizerItemFilter::UnionFilter) {
        QOrganizerItemUnionFilter bf(left);
        /* we can just add the right to this one */
        bf.append(right);
        return bf;
    }

    if (right.type() == QOrganizerItemFilter::UnionFilter) {
        QOrganizerItemUnionFilter bf(right);
        /* we can prepend the left to this one */
        bf.prepend(left);
        return bf;
    }

    /* usual fallback case */
    QOrganizerItemUnionFilter nif;
    nif << left << right;
    return nif;
}

QT_END_NAMESPACE_ORGANIZER
