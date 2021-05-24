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

#include <tracker-sparql.h>

#include "qgallerytrackerschema_p.h"

#include "qgalleryabstractrequest.h"
#include "qgallerytrackerresultset_p.h"
#include "qgallerytrackerlistcolumn_p.h"

#include <QtCore/qdatetime.h>
#include <QtCore/qdir.h>
#include <QtCore/qmetatype.h>
#include <QtCore/qsettings.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qurl.h>
#include <QtCore/qxmlstream.h>
#include <QtCore/qdebug.h>

#include <tracker-sparql.h>

QT_BEGIN_NAMESPACE_DOCGALLERY

namespace
{

    template <typename T>
    struct QGalleryPropertyList
    {
        QGalleryPropertyList() : items(0), count(0) {}
        template <int N> QGalleryPropertyList(const T (&items)[N])
            : items(items), count(N) {}

        const T *items;
        const int count;

        int indexOfProperty(const QString &propertyName) const;

        const T &operator [](int index) const { return items[index]; }
    };


    struct QGalleryItemProperty
    {
        QLatin1String name;
        QLatin1String field;
        QLatin1String join;
        QVariant::Type type;
        QGalleryProperty::Attributes attributes;
    };

    typedef QGalleryPropertyList<QGalleryItemProperty> QGalleryItemPropertyList;

    struct QGalleryCompositeProperty
    {
        QLatin1String name;
        QVariant::Type type;
        QGalleryItemPropertyList dependencies;
        QGalleryTrackerCompositeColumn *(*createColumn)(const QVector<int> &columns);
        bool (*writeFilterCondition)(
                QDocumentGallery::Error *error,
                QString *query,
                const QGalleryCompositeProperty &property,
                const QGalleryMetaDataFilter &filter);
    };

    typedef QGalleryPropertyList<QGalleryCompositeProperty> QGalleryCompositePropertyList;

    enum UpdateId
    {
        FileId          = 0x0001,
        FolderId        = 0x0002,
        DocumentId      = 0x0004,
        AudioId         = 0x0008,
        ImageId         = 0x0010,
        VideoId         = 0x0020,
        PlaylistId      = 0x0040,
        TextId          = 0x0080,
        ArtistId = 0x0100,
        AlbumId = 0x0200,
        AlbumArtistId = ArtistId,
        PhotoAlbumId = 0x0400,
        AudioGenreId = 0x0000

    };

    enum UpdateMask
    {
        FileMask        = FileId
                        | FolderId
                        | DocumentId
                        | AudioId
                        | ImageId
                        | VideoId
                        | PlaylistId
                        | TextId,
        FolderMask      = FolderId,
        DocumentMask    = DocumentId,
        AudioMask       = AudioId,
        ImageMask       = ImageId,
        VideoMask       = VideoId,
        PlaylistMask    = PlaylistId,
        TextMask        = TextId,
        ArtistMask = ArtistId,
        AlbumMask = AlbumId,
        AlbumArtistMask = AlbumArtistId,
        PhotoAlbumMask = PhotoAlbumId,
        AudioGenreMask = AudioMask

    };

    struct QGalleryTypePrefix : public QLatin1String
    {
        template <int N> QGalleryTypePrefix(const char (&prefix)[N])
            : QLatin1String(prefix), length(N - 1){}

        const int length;

        QStringRef strip(const QString &id) const {
            return QStringRef(&id, length, id.length() - length); }
    };

    struct QGalleryItemType
    {
        QLatin1String itemType;
        QLatin1String service;
        QLatin1String identity;
        QLatin1String rdfSuffix;
        QLatin1String typeFragment;
        const char *filterFragment;
        QGalleryTypePrefix prefix;
        QGalleryItemPropertyList itemProperties;
        QGalleryCompositePropertyList compositeProperties;
        int updateId;
        int updateMask;
    };

    template <typename T>
    struct QGalleryTypeList
    {
        template <int N> QGalleryTypeList(const T (&items)[N])
            : items(items), count(N) {}

        const T *items;
        const int count;

        int indexOfType(const QString &type) const;
        int indexOfItemId(const QString &itemId) const;
        int indexOfService(const QString &service) const;
        int indexOfRdfTypes(const QStringList &rdfTypes) const;
        const T &operator [](int index) const { return items[index]; }
    };

    typedef QGalleryTypeList<QGalleryItemType> QGalleryItemTypeList;

    template <typename T>
    int QGalleryPropertyList<T>::indexOfProperty(const QString &name) const
    {
        for (int i = 0; i < count; ++i) {
            if (items[i].name == name)
                return i;
        }
        return -1;
    }

    template <typename T>
    int QGalleryTypeList<T>::indexOfType(const QString &type) const
    {
        for (int i = 0; i < count; ++i) {
            if (items[i].itemType == type)
                return i;
        }
        return -1;
    }

    template <typename T>
    int QGalleryTypeList<T>::indexOfItemId(const QString &itemId) const
    {
        for (int i = 0; i < count; ++i) {
            if (itemId.startsWith(items[i].prefix))
                return i;
        }
        return -1;
    }

    template <typename T>
    int QGalleryTypeList<T>::indexOfService(const QString &service) const
    {
        for (int i = 0; i < count; ++i) {
            if (items[i].service == service)
                return i;
        }
        return -1;
    }

    template <typename T>
    int QGalleryTypeList<T>::indexOfRdfTypes(const QStringList &rdfTypes) const
    {
        int index = -1;
        int rdfIndex = -1;
        for (int i = 0; i < count; ++i) {
            for (int j = rdfTypes.count() - 1; j >= 0; --j) {
                if (rdfTypes.at(j).endsWith(items[i].rdfSuffix)) {
                    if (j > rdfIndex) {
                        index = i;
                        rdfIndex = j;
                    }
                    break;
                }
            }
        }
        return index;
    }
    // Re-declare to cut down on prefixes.
    enum
    {
        CanRead         = QGalleryProperty::CanRead,
        CanWrite        = QGalleryProperty::CanWrite,
        CanSort         = QGalleryProperty::CanSort,
        CanFilter       = QGalleryProperty::CanFilter,
        IsResource      = 0x100,
        PropertyMask    = 0xFF
    };
}

#define QT_GALLERY_ITEM_PROPERTY(PropertyName, Field, Type, Attr) \
{ QLatin1String(PropertyName), QLatin1String(Field), QLatin1String(""), QVariant::Type, QGalleryProperty::Attributes(Attr) }

#define QT_GALLERY_LINKED_PROPERTY(PropertyName, Field, Join, Type, Attr) \
{ QLatin1String(PropertyName), QLatin1String(Field), QLatin1String(Join), QVariant::Type, QGalleryProperty::Attributes(Attr) }

#define QT_GALLERY_COMPOSITE_PROPERTY(PropertyName, Type, Dependencies, Factory, QueryBuilder) \
{ QLatin1String(PropertyName), QVariant::Type, QGalleryItemPropertyList(Dependencies), Factory, QueryBuilder }

#define QT_GALLERY_COMPOSITE_PROPERTY_NO_DEPENDENCIES(PropertyName, Type, Factory, QueryBuilder) \
{ QLatin1String(PropertyName), QVariant::Type, QGalleryItemPropertyList(), Factory, QueryBuilder }

#define QT_GALLERY_ITEM_TYPE(Type, RdfPrefix, RdfType, Prefix, PropertyGroup) \
{ \
    QLatin1String(#Type), \
    QLatin1String(#RdfPrefix":"#RdfType), \
    QLatin1String("?x"), \
    QLatin1String("/"#RdfPrefix"#"#RdfType), \
    QLatin1String("?x a "#RdfPrefix":"#RdfType" . ?x tracker:available true"), \
    0, \
    QGalleryTypePrefix(#Prefix"::"), \
    QGalleryItemPropertyList(qt_gallery##PropertyGroup##PropertyList), \
    QGalleryCompositePropertyList(qt_gallery##PropertyGroup##CompositePropertyList), \
    Type##Id, \
    Type##Mask \
}

#define QT_GALLERY_ITEM_TYPE_NO_COMPOSITE(Type, RdfPrefix, RdfType, Prefix, PropertyGroup) \
{ \
    QLatin1String(#Type), \
    QLatin1String(#RdfPrefix":"#RdfType), \
    QLatin1String("?x"), \
    QLatin1String("/"#RdfPrefix"#"#RdfType), \
    QLatin1String("?x a "#RdfPrefix":"#RdfType), \
    0, \
    QGalleryTypePrefix(#Prefix"::"), \
    QGalleryItemPropertyList(qt_gallery##PropertyGroup##PropertyList), \
    QGalleryCompositePropertyList(), \
    Type##Id, \
    Type##Mask \
}

#define QT_GALLERY_ITEM_TYPE_NO_COMPOSITE_FILTERED(Type, RdfPrefix, RdfType, Filter, Prefix, PropertyGroup) \
{ \
    QLatin1String(#Type), \
    QLatin1String(#RdfPrefix":"#RdfType), \
    QLatin1String("?x"), \
    QLatin1String("/"#RdfPrefix"#"#RdfType), \
    QLatin1String("?x a "#RdfPrefix":"#RdfType Filter), \
    0, \
    QGalleryTypePrefix(#Prefix"::"), \
    QGalleryItemPropertyList(qt_gallery##PropertyGroup##PropertyList), \
    QGalleryCompositePropertyList(), \
    Type##Id, \
    Type##Mask \
}

#define QT_GALLERY_AGGREGATE_TYPE_NO_COMPOSITE(Type, RdfPrefix, RdfType, Identity, Prefix, PropertyGroup) \
{ \
    QLatin1String(#Type), \
    QLatin1String(#RdfPrefix":"#RdfType), \
    QLatin1String(#Identity), \
    QLatin1String("/"#RdfPrefix"#"#RdfType), \
    QLatin1String("?x a "#RdfPrefix":"#RdfType " . ?x tracker:available true"), \
    #Identity"!=''", \
    QGalleryTypePrefix(#Prefix"::"), \
    QGalleryItemPropertyList(qt_gallery##PropertyGroup##PropertyList), \
    QGalleryCompositePropertyList(), \
    Type##Id, \
    Type##Mask \
}


static void qt_appendJoin(QString *currentJoin, const QString &typeJoin, const QString &join)
{
    int end = 0;
    int begin;
    do {
        begin = end;
        end = join.indexOf(QLatin1String(" ."), begin + 2);

        const QString substring = join.mid(begin, end != -1 ? end - begin : -1);
        if (!typeJoin.contains(substring) && !currentJoin->contains(substring))
            *currentJoin += substring;
    } while (end != -1);
}

static bool qt_writeCondition(
        QDocumentGallery::Error *error,
        QString *query,
        QString *join,
        const QString &typeJoin,
        const QGalleryFilter &filter,
        const QGalleryItemPropertyList &properties,
        const QGalleryCompositePropertyList &composites);

static bool qt_writeConditionHelper(
        QDocumentGallery::Error *error,
        QString *query,
        QString *join,
        const QString &typeJoin,
        const QList<QGalleryFilter> &filters,
        const QGalleryItemPropertyList &properties,
        const QGalleryCompositePropertyList &composites,
        const QString& op
        )
{
    if (!filters.isEmpty()) {
        *query += QLatin1Char('(');

        int count = filters.size();
        for (QList<QGalleryFilter>::const_iterator it = filters.begin(), end = filters.end();
                it != end;
                ++it) {
            if (!qt_writeCondition(error, query, join, typeJoin, *it, properties, composites))
                return false;
            if ( --count > 0 )
                *query += op;
        }
        *query += QLatin1Char(')');
    }
    return true;
}

static bool qt_writeCondition(
        QDocumentGallery::Error *error,
        QString *query,
        QString *join,
        const QString &typeJoin,
        const QGalleryIntersectionFilter &filter,
        const QGalleryItemPropertyList &properties,
        const QGalleryCompositePropertyList &composites)
{
    return qt_writeConditionHelper(error, query, join, typeJoin, filter.filters(), properties, composites, QLatin1String("&&"));
}

static bool qt_writeCondition(
        QDocumentGallery::Error *error,
        QString *query,
        QString *join,
        const QString &typeJoin,
        const QGalleryUnionFilter &filter,
        const QGalleryItemPropertyList &properties,
        const QGalleryCompositePropertyList &composites)
{
    return qt_writeConditionHelper(error, query, join, typeJoin, filter.filters(), properties, composites, QLatin1String("||"));
}

static bool qt_write_comparison(
        QDocumentGallery::Error *error,
        const QLatin1String &field,
        const QVariant &value,
        const char *op,
        QString *query,
        QVariant::Type type = QVariant::String)
{
    QString stringValue;
    if (type == QVariant::Url && value.canConvert(QVariant::Url)) {
        QByteArray encodedUrl = value.toUrl().toEncoded();
        stringValue = QString::fromUtf8(encodedUrl.data(), encodedUrl.length());
    } else if (value.canConvert(QVariant::String)) {
        stringValue = value.toString();
    } else {
        *error = QDocumentGallery::FilterError;
        return false;
    }

    *query += QLatin1String("(")
            + field
            + QLatin1String(op)
            + QLatin1String("'")
            + stringValue
            + QLatin1String("')");

    return true;
}

static bool qt_write_function(
        QDocumentGallery::Error *,
        const char *function,
        const QString &field,
        const QRegExp &regExp,
        QString *query)
{
    *query += QLatin1String(function)
            + QLatin1String("(")
            + field
            + QLatin1String(",'")
            + regExp.pattern()
            + QLatin1String("')");
    return true;
}

static bool qt_write_function(
        QDocumentGallery::Error *error,
        const char *function,
        const QString &field,
        const QVariant &value,
        QString *query,
        QVariant::Type type = QVariant::String)
{
    QString stringValue;
    if (type == QVariant::Url && value.canConvert(QVariant::Url)) {
        QByteArray encodedUrl = value.toUrl().toEncoded();
        stringValue = QString::fromUtf8(encodedUrl.data(), encodedUrl.length());
    } else if (value.canConvert(QVariant::String)) {
        stringValue = value.toString();
    } else {
        *error = QDocumentGallery::FilterError;
        return false;
    }

    *query += QLatin1String(function)
            + QLatin1String("(")
            + field
            + QLatin1String(",'")
            + stringValue
            + QLatin1String("')");

        return true;
}

static bool qt_writeCondition(
        QDocumentGallery::Error *error,
        QString *query,
        QString *join,
        const QString &typeJoin,
        const QGalleryMetaDataFilter &filter,
        const QGalleryItemPropertyList &properties,
        const QGalleryCompositePropertyList &composites)
{
    if (filter.isNegated())
        *query += QLatin1Char('!');

    const QString propertyName = filter.propertyName();

    int index;

    if ((index = properties.indexOfProperty(propertyName)) != -1) {
        const QVariant value = filter.value();
        const QGalleryItemProperty &property = properties[index];

        if (property.join != QLatin1String(""))
            qt_appendJoin(join, typeJoin, property.join);

        switch (filter.comparator()) {
        case QGalleryFilter::Equals:
            return value.type() != QVariant::RegExp
                    ? qt_write_comparison(error, property.field, value, "=", query, property.type)
                    : qt_write_function(error, "REGEX", properties[index].field, value.toRegExp(), query);
        case QGalleryFilter::LessThan:
            return qt_write_comparison(error, property.field, value, "<", query, property.type);
        case QGalleryFilter::GreaterThan:
            return qt_write_comparison(error, property.field, value, ">", query, property.type);
        case QGalleryFilter::LessThanEquals:
            return qt_write_comparison(error, property.field, value, "<=", query, property.type);
        case QGalleryFilter::GreaterThanEquals:
            return qt_write_comparison(error, property.field, value, ">=", query, property.type);
        case QGalleryFilter::Contains:
            return  qt_write_function(error, "fn:contains", property.field, value, query, property.type);
        case QGalleryFilter::StartsWith:
            return qt_write_function(error, "fn:starts-with", property.field, value, query, property.type);
        case QGalleryFilter::EndsWith:
            return qt_write_function(error, "fn:ends-with", property.field, value, query, property.type);
        case QGalleryFilter::Wildcard:
            return qt_write_function(error, "fn:contains", property.field, value, query, property.type);
        case QGalleryFilter::RegExp:
            return value.type() != QVariant::RegExp
                    ? qt_write_function(error, "REGEX", property.field, value, query, property.type)
                    : qt_write_function(error, "REGEX", property.field, value.toRegExp(), query);
        default:
            *error = QDocumentGallery::FilterError;

            return false;
        }
        return true;
    } else if ((index = composites.indexOfProperty(propertyName)) != -1
            && composites[index].writeFilterCondition) {
        return composites[index].writeFilterCondition(error, query, composites[index], filter);
    } else {
        *error = QDocumentGallery::FilterError;
        return false;
    }
}

static bool qt_writeCondition(
        QDocumentGallery::Error *error,
        QString *query,
        QString *join,
        const QString &typeJoin,
        const QGalleryFilter &filter,
        const QGalleryItemPropertyList &properties,
        const QGalleryCompositePropertyList &composites)
{
    switch (filter.type()) {
    case QGalleryFilter::Intersection:
        return qt_writeCondition(
                error, query, join, typeJoin, filter.toIntersectionFilter(), properties, composites);
    case QGalleryFilter::Union:
        return qt_writeCondition(error, query, join, typeJoin, filter.toUnionFilter(), properties, composites);
    case QGalleryFilter::MetaData:
        return qt_writeCondition(error, query, join, typeJoin, filter.toMetaDataFilter(), properties, composites);
    default:
        Q_ASSERT(filter.type() != QGalleryFilter::Invalid);
        *error = QDocumentGallery::FilterError;
        return false;
    }
}

static QString qt_encodedFilePathUrl(const QString &filePath)
{
    QString encodedUrl = QUrl::fromLocalFile(filePath).toString(QUrl::FullyEncoded);
    encodedUrl.replace(QLatin1Char('\''), QLatin1String("\\\'"));
    return encodedUrl;
}

static QString qt_encodedFilePathFragment(const QString &fragment)
{
    QString encodedFragment = QUrl(fragment).toString(QUrl::FullyEncoded);
    encodedFragment.replace(QLatin1Char('\''), QLatin1String("\\\'"));
    return encodedFragment;
}

static bool qt_writeFilePathUrlCondition(
        QDocumentGallery::Error *error,
        QString *query,
        const QLatin1String &property,
        const QGalleryMetaDataFilter &filter)
{
    const QVariant value = filter.value();

    if (value.type() != QVariant::String) {
        *error = QDocumentGallery::FilterError;
        return false;
    } else {
        const QString &filePath = value.toString();

        switch (filter.comparator()) {
        case QGalleryFilter::Equals:
            return qt_write_comparison(
                    error, property, qt_encodedFilePathUrl(filePath), "=", query);
        case QGalleryFilter::LessThan:
            return qt_write_comparison(
                    error, property, qt_encodedFilePathUrl(filePath), "<", query);
        case QGalleryFilter::GreaterThan:
            return qt_write_comparison(
                    error, property, qt_encodedFilePathUrl(filePath), ">", query);
        case QGalleryFilter::LessThanEquals:
            return qt_write_comparison(
                    error, property, qt_encodedFilePathUrl(filePath), "<=", query);
        case QGalleryFilter::GreaterThanEquals:
            return qt_write_comparison(
                    error, property, qt_encodedFilePathUrl(filePath), ">=", query);
        case QGalleryFilter::Contains:
            return  qt_write_function(
                    error, "fn:contains", property, qt_encodedFilePathFragment(filePath), query);
        case QGalleryFilter::StartsWith:
            return qt_write_function(
                    error, "fn:starts-with", property, qt_encodedFilePathUrl(filePath), query);
        case QGalleryFilter::EndsWith:
            return qt_write_function(
                    error, "fn:ends-with", property, qt_encodedFilePathFragment(filePath), query);
        case QGalleryFilter::Wildcard:
            return qt_write_function(
                    error, "fn:contains", property, qt_encodedFilePathUrl(filePath), query);
        case QGalleryFilter::RegExp:    // Unsupported.
        default:
            *error = QDocumentGallery::FilterError;
            return false;
        }
    }
}

static bool qt_writeFilePathCondition(
        QDocumentGallery::Error *error,
        QString *query,
        const QGalleryCompositeProperty &,
        const QGalleryMetaDataFilter &filter)
{
    return qt_writeFilePathUrlCondition(error, query, QLatin1String("nie:url(?x)"), filter);
}

static bool qt_writePathCondition(
        QDocumentGallery::Error *error,
        QString *query,
        const QGalleryCompositeProperty &,
        const QGalleryMetaDataFilter &filter)
{
    return qt_writeFilePathUrlCondition(
            error, query, QLatin1String("nie:url(nfo:belongsToContainer(?x))"), filter);
}

static bool qt_writeFileExtensionCondition(
        QDocumentGallery::Error *error,
        QString *query,
        const QGalleryCompositeProperty &,
        const QGalleryMetaDataFilter &filter)
{
    if (filter.comparator() != QGalleryFilter::Equals || filter.value().type() != QVariant::String) {
        *error = QDocumentGallery::FilterError;
        return false;
    } else {
        *query += QLatin1String("fn:ends-with(nfo:fileName(?x),'.")
                + filter.value().toString()
                + QLatin1String("')");
        return true;
    }
}


static bool qt_writeOrientationCondition(
        QDocumentGallery::Error *error,
        QString *query,
        const QGalleryCompositeProperty &,
        const QGalleryMetaDataFilter &filter)
{
    if (filter.comparator() != QGalleryFilter::Equals || filter.value().type() != QVariant::Int) {
        *error = QDocumentGallery::FilterError;
        return false;
    } else switch (filter.value().toInt()) {
    case 0:
        *query += QLatin1String("nfo:orientation(?x) = 'http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#orientation-top'");
        return true;
    case 90:
        *query += QLatin1String("nfo:orientation(?x) = 'http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#orientation-left'");
        return true;
    case 180:
        *query += QLatin1String("nfo:orientation(?x) = 'http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#orientation-bottom'");
        return true;
    case 270:
        *query += QLatin1String("nfo:orientation(?x) = 'http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#orientation-right'");
        return true;
    default:
        *error = QDocumentGallery::FilterError;
        return false;
    }
}

//nie:DataObject
//  nie:url nie:isPartOf, nie:created, nie:lastRefreshed, nie:interpretedAs, nie:dataSource,
//  nie:byteSize
#define QT_GALLERY_NIE_DATAOBJECT_PROPERTIES \
    QT_GALLERY_ITEM_PROPERTY("url", "nie:url(?x)", Url, CanRead | CanSort | CanFilter | IsResource)

//nie:InformationElement
//  nie:usageCounter, nie:rootElementOf, nie:contentSize, nie:isLogicalPartOf, nie:characterSet,
//  nie:licenseType, nie:hasPart, nie:hasLogicalPart, nie:keyword, nie:identifier, nie:license,
//  nie:contentAccessed, nie:contentCreated, nie:version, nie:contentLastModified, nie:isStoredAs,
//  nie:comment, nie:copyright, nie:links, nie:depends, nie:disclaimer, nie:description,
//  nie:generator, nie:relatedTo, nie:legal, nie:informationElementDate, nie:plainTextContent,
//  nie:language, nie:mimeType, nie:subject, nie:title
#define QT_GALLERY_NIE_INFORMATIONELEMENT_PROPERTIES \
    QT_GALLERY_ITEM_PROPERTY("author"     , "dc:creator(?x)"       , String    , CanRead | CanWrite | CanSort | CanFilter), \
    QT_GALLERY_ITEM_PROPERTY("comments"   , "nie:comment(?x)"      , String    , CanRead | CanWrite | CanSort | CanFilter), \
    QT_GALLERY_ITEM_PROPERTY("copyright"  , "nie:copyright(?x)"    , String    , CanRead | CanWrite | CanSort | CanFilter), \
    QT_GALLERY_ITEM_PROPERTY("description", "nie:description(?x)"  , String    , CanRead | CanWrite | CanSort | CanFilter), \
    QT_GALLERY_ITEM_PROPERTY("keywords"   , "nie:keyword(?x)"      , StringList, CanRead | CanWrite | CanSort | CanFilter), \
    QT_GALLERY_ITEM_PROPERTY("language"   , "nie:language(?x)"     , String    , CanRead | CanWrite | CanSort | CanFilter), \
    QT_GALLERY_ITEM_PROPERTY("mimeType"   , "nie:mimeType(?x)"     , String    , CanRead | CanSort | CanFilter | IsResource), \
    QT_GALLERY_ITEM_PROPERTY("rating"     , "nao:numericRating(?x)", Double    , CanRead | CanWrite | CanSort | CanFilter), \
    QT_GALLERY_ITEM_PROPERTY("subject"    , "nie:subject(?x)"      , String    , CanRead | CanWrite | CanSort | CanFilter), \
    QT_GALLERY_ITEM_PROPERTY("title"      , "nie:title(?x)"        , String    , CanRead | CanWrite | CanSort | CanFilter)

//nfo:FileDataObject : nie:DataObject, nie:InformationElement
//  nfo:fileLastModified, nfo:fileOwner, nfo:hasHash, nfo:fileUrl, nfo:fileName, nfo:permissions,
//  nfo:fileSize, nfo:fileCreated, nfo:fileLastAccessed
#define QT_GALLERY_NFO_FILEDATAOBJECT_PROPERTIES \
    QT_GALLERY_NIE_DATAOBJECT_PROPERTIES, \
    QT_GALLERY_NIE_INFORMATIONELEMENT_PROPERTIES, \
    QT_GALLERY_ITEM_PROPERTY("fileName"     , "nfo:fileName(?x)"         , String  , CanRead | CanSort | CanFilter), \
    QT_GALLERY_ITEM_PROPERTY("fileSize"     , "nfo:fileSize(?x)"         , Int     , CanRead | CanSort | CanFilter), \
    QT_GALLERY_ITEM_PROPERTY("lastAccessed" , "nfo:fileLastAccessed(?x)" , DateTime, CanRead | CanSort | CanFilter), \
    QT_GALLERY_ITEM_PROPERTY("lastModified" , "nfo:fileLastModified(?x)" , DateTime, CanRead | CanSort | CanFilter)


static const QGalleryItemProperty qt_galleryOrientationPropertyList[] = {
    QT_GALLERY_ITEM_PROPERTY("_orientation", "nfo:orientation(?x)", String, CanRead | CanFilter)
};

#define QT_GALLERY_NFO_FILEDATAOBJECT_COMPOSITE_PROPERTIES \
    QT_GALLERY_COMPOSITE_PROPERTY_NO_DEPENDENCIES("fileExtension", String, QGalleryTrackerFileExtensionColumn::create, qt_writeFileExtensionCondition), \
    QT_GALLERY_COMPOSITE_PROPERTY_NO_DEPENDENCIES("filePath"     , String, QGalleryTrackerFilePathColumn::create     , qt_writeFilePathCondition), \
    QT_GALLERY_COMPOSITE_PROPERTY_NO_DEPENDENCIES("path"         , String, QGalleryTrackerPathColumn::create         , qt_writePathCondition)

//nfo:Media : nfo:FileDataObject
//  nfo:equipment, nfo:genre, nfo:averageBitrate, nfo:bitrateType, nfo:encodedBy, nfo:codec,
//  nfo:bitDepth, nfo:hasMediaStream, nfo:compressionType, nfo:device, nfo:duration, nfo:count

#define QT_GALLERY_NFO_MEDIA_PROPERTIES \
    QT_GALLERY_NFO_FILEDATAOBJECT_PROPERTIES, \
    QT_GALLERY_ITEM_PROPERTY("audioBitRate"  , "nfo:averageAudioBitrate(?x)"      , Int     , CanRead | CanSort | CanFilter | IsResource), \
    QT_GALLERY_ITEM_PROPERTY("audioCodec"    , "nfo:codec(?x)"                    , String  , CanRead | CanSort | CanFilter | IsResource), \
    QT_GALLERY_ITEM_PROPERTY("channelCount"  , "nfo:channels(?x)"                 , Int     , CanRead | CanSort | CanFilter | IsResource), \
    QT_GALLERY_ITEM_PROPERTY("duration"      , "nfo:duration(?x)"                 , Int     , CanRead | CanSort | CanFilter | IsResource), \
    QT_GALLERY_ITEM_PROPERTY("lastPlayed"    , "nie:contentAccessed(?x)"          , DateTime, CanRead | CanSort | CanFilter), \
    QT_GALLERY_ITEM_PROPERTY("playCount"     , "nie:usageCounter(?x)"             , Int     , CanRead | CanWrite | CanSort | CanFilter), \
    QT_GALLERY_ITEM_PROPERTY("sampleRate"    , "nfo:sampleRate(?x)"               , Int     , CanRead | CanSort | CanFilter | IsResource), \
    QT_GALLERY_LINKED_PROPERTY("performer", "nmm:artistName(?artist)", " . ?x nmm:performer ?artist", String  , CanRead | CanSort | CanFilter)

//nfo:Visual : nfo:Media
//  nfo:colorDepth, nfo:width, nfo:height, nfo:interlaceMode, nfo:tilt, nfo:heading, nfo:aspectRatio

#define QT_GALLERY_NFO_VISUAL_PROPERTIES \
    QT_GALLERY_ITEM_PROPERTY("height", "nfo:height(?x)", Int, CanRead | CanSort | CanFilter | IsResource), \
    QT_GALLERY_ITEM_PROPERTY("width" , "nfo:width(?x)" , Int, CanRead | CanSort | CanFilter | IsResource), \
    QT_GALLERY_LINKED_PROPERTY("latitude" , "slo:latitude(?location)" , " . ?x slo:location ?location", Double, CanRead | CanFilter | CanSort), \
    QT_GALLERY_LINKED_PROPERTY("longitude", "slo:longitude(?location)", " . ?x slo:location ?location", Double, CanRead | CanFilter | CanSort), \
    QT_GALLERY_LINKED_PROPERTY("altitude" , "slo:altitude(?location)" , " . ?x slo:location ?location", Double, CanRead | CanFilter | CanSort)

#define QT_GALLERY_NFO_VISUAL_COMPOSITE_PROPERTIES \
    QT_GALLERY_COMPOSITE_PROPERTY("orientation", Int, qt_galleryOrientationPropertyList, QGalleryTrackerOrientationColumn::create, qt_writeOrientationCondition)
///////
// File
///////

//nfo:DataContainer : nie:InformationElement

//nfo:Folder : nfo:FileDataObject, nfo:DataContainer

static const QGalleryItemProperty qt_galleryFilePropertyList[] =
{
    QT_GALLERY_NFO_MEDIA_PROPERTIES,
    QT_GALLERY_NFO_VISUAL_PROPERTIES
};

#define QT_GALLERY_FILE_COMPOSITE_PROPERTIES \
    QT_GALLERY_COMPOSITE_PROPERTY_NO_DEPENDENCIES("filePath", String, QGalleryTrackerFilePathColumn::create, qt_writeFilePathCondition)

    static const QGalleryCompositeProperty qt_galleryFileCompositePropertyList[] =
{
    QT_GALLERY_NFO_FILEDATAOBJECT_COMPOSITE_PROPERTIES,
    QT_GALLERY_NFO_VISUAL_COMPOSITE_PROPERTIES
};

////////
// Audio
////////

//nfo:Audio : nfo:Media
//  nfo:peakGain, nfo:gain, nfo:rearChannels, nfo:averageAudioBitrate, nfo:sampleRate,
//  nfo:frontChannels, nfo:bitsPerSample, nfo:sampleCount, nfo:lfeChannels, nfo:sideChannels,
//  nfo:channels

//nmm:MusicPiece : nfo:Audio
//  nmm:internationalStandardRecordingCode, nmm:trackNumber, nmm:lyrics, nmm:lyricist, nmm:composer
//  nmm:length, nmm:performer, nmm:beatsPerMinute, nmm:musicAlbumDisc, nmm:musicAlbum,

static const QGalleryItemProperty qt_galleryAudioPropertyList[] =
{
    QT_GALLERY_NFO_MEDIA_PROPERTIES,
    QT_GALLERY_ITEM_PROPERTY("genre"      , "nfo:genre(?x)"                                      , String, CanRead | CanWrite | CanSort | CanFilter),
    QT_GALLERY_ITEM_PROPERTY("lyrics"     , "nmm:lyrics(?x)"                                     , String, CanRead | CanWrite | CanSort | CanFilter),
    QT_GALLERY_ITEM_PROPERTY("trackNumber", "nmm:trackNumber(?x)"                                , Int   , CanRead | CanWrite | CanSort | CanFilter),
    QT_GALLERY_LINKED_PROPERTY("discNumber" , "nmm:setNumber(?disc)"        , " . ?x nmm:musicAlbumDisc ?disc"                                   , Int   , CanRead | CanSort | CanFilter),
    QT_GALLERY_LINKED_PROPERTY("artist"     , "nmm:artistName(?artist)"     , " . ?x nmm:performer ?artist"                                      , String, CanRead | CanSort | CanFilter),
    QT_GALLERY_LINKED_PROPERTY("composer"   , "nmm:artistName(?composer)"   , " . ?x nmm:composer ?composer"                                     , String, CanRead | CanSort | CanFilter),
    QT_GALLERY_LINKED_PROPERTY("albumArtist", "nmm:artistName(?albumArtist)", " . ?x nmm:musicAlbum ?album . ?album nmm:albumArtist ?albumArtist", String, CanRead | CanSort | CanFilter),
    QT_GALLERY_LINKED_PROPERTY("albumTitle" , "nmm:albumTitle(?album)"      , " . ?x nmm:musicAlbum ?album"                                      , String, CanRead | CanSort | CanFilter)
};

static const QGalleryCompositeProperty qt_galleryAudioCompositePropertyList[] =
{
    QT_GALLERY_NFO_FILEDATAOBJECT_COMPOSITE_PROPERTIES
};

///////////
// Playlist
///////////

//nfo:MediaList : nie:InformationElement
//  nfo:mediaListEntry, nfo:listDuration, nfo:entryCounter, nfo:hasMediaFileListEntry

//nmm:Playlist : nfo:FileDataObject, nfo:MediaList

static const QGalleryItemProperty qt_galleryPlaylistPropertyList[] =
{
    QT_GALLERY_NFO_FILEDATAOBJECT_PROPERTIES,
    QT_GALLERY_ITEM_PROPERTY("duration"  , "nfo:listDuration(?x)", Int, CanRead | CanSort | CanFilter),
    QT_GALLERY_ITEM_PROPERTY("trackCount", "nfo:entryCounter(?x)", Int, CanRead | CanSort | CanFilter)
};

static const QGalleryCompositeProperty qt_galleryPlaylistCompositePropertyList[] =
{
    QT_GALLERY_NFO_FILEDATAOBJECT_COMPOSITE_PROPERTIES
};

////////
// Image
////////

//nfo:Image : nfo:Visual
//  nfo:depicts, nfo:orientation, nfo:horizontalResolution, nfo:verticalResolution

//nmm:Photo : nfo:Image
//  nmm:isColorCorrected, nmm:isCropped, nmm:whiteBalance, nmm:meteringMode, nmm:isoSpeed,
//  nmm:focalLength, nmm:fnumber, nmm:flash, nmm:exposureTime, nmm:camera,

static const QGalleryItemProperty qt_galleryImagePropertyList[] =
{
    QT_GALLERY_NFO_FILEDATAOBJECT_PROPERTIES,
    QT_GALLERY_NFO_VISUAL_PROPERTIES,
    QT_GALLERY_ITEM_PROPERTY("exposureTime"      , "nmm:exposureTime(?x)"               , String  , CanRead | CanWrite | CanSort | CanFilter),
    QT_GALLERY_ITEM_PROPERTY("dateTaken"         , "nie:contentCreated(?x)"             , DateTime, CanRead | CanWrite | CanSort | CanFilter),
    QT_GALLERY_ITEM_PROPERTY("fNumber"           , "nmm:fnumber(?x)"                    , Double  , CanRead | CanWrite | CanSort | CanFilter),
    QT_GALLERY_ITEM_PROPERTY("flashEnabled"      , "nmm:flash(?x)"                      , String  , CanRead | CanWrite | CanSort | CanFilter),
    QT_GALLERY_ITEM_PROPERTY("focalLength"       , "nmm:focalLength(?x)"                , Double  , CanRead | CanWrite | CanSort | CanFilter),
    QT_GALLERY_ITEM_PROPERTY("meteringMode"      , "nmm:meteringMode(?x)"               , String  , CanRead | CanWrite | CanSort | CanFilter),
    QT_GALLERY_ITEM_PROPERTY("whiteBalance"      , "nmm:whiteBalance(?x)"               , String  , CanRead | CanWrite | CanSort | CanFilter),
    QT_GALLERY_LINKED_PROPERTY("cameraManufacturer", "nfo:manufacturer(?camera)", " . ?x nfo:equipment ?camera", String  , CanRead | CanSort | CanFilter),
    QT_GALLERY_LINKED_PROPERTY("cameraModel"       , "nfo:model(?camera)"       , " . ?x nfo:equipment ?camera", String  , CanRead | CanSort | CanFilter)
};

static const QGalleryCompositeProperty qt_galleryImageCompositePropertyList[] =
{
    QT_GALLERY_NFO_FILEDATAOBJECT_COMPOSITE_PROPERTIES,
    QT_GALLERY_NFO_VISUAL_COMPOSITE_PROPERTIES
};

////////
// Video
////////

//nfo:Video : nfo:Visual
//  nfo:averageVideoBitrate, nfo:frameCount, nfo:frameRate

//nmm:Video : nfo:Video
//  nmm:isContentEncrypted, nmm:subtitle, nmm:hasSubtitle, nmm:leadActor, nmm:producedBy,
//  nmm:director, nmm:category, nmm:MPAARating, nmm:synopsis, nmm:runTime, nmm:episodeNumber
//  nmm:season, nmm:isSeries, nmm:videoAlbum,

static const QGalleryItemProperty qt_galleryVideoPropertyList[] =
{
    QT_GALLERY_NFO_MEDIA_PROPERTIES,
    QT_GALLERY_NFO_VISUAL_PROPERTIES,
    QT_GALLERY_ITEM_PROPERTY("frameRate"     , "nfo:frameRate(?x)"                 , Double, CanRead | CanSort  | CanFilter | IsResource),
    QT_GALLERY_ITEM_PROPERTY("resumePosition", "nfo:streamPosition(?x)"            , Int   , CanRead | CanWrite | CanSort   | CanFilter),
    QT_GALLERY_ITEM_PROPERTY("videoCodec"    , "nfo:codec(?x)"                     , String, CanRead | CanSort  | CanFilter | IsResource),
    QT_GALLERY_ITEM_PROPERTY("videoBitRate"  , "nfo:averageBitrate(?x)"            , Int   , CanRead | CanSort  | CanFilter | IsResource),
    QT_GALLERY_LINKED_PROPERTY("director"      , "nmm:artistName(director)", " . ?x nmm:directory ?director"  , String, CanRead | CanSort  | CanFilter | IsResource),
    QT_GALLERY_LINKED_PROPERTY("producer"      , "nmm:artistName(producer)", " . ?x nmm:producedBy ? producer", String, CanRead | CanSort  | CanFilter | IsResource)
};

static const QGalleryCompositeProperty qt_galleryVideoCompositePropertyList[] =
{
    QT_GALLERY_NFO_FILEDATAOBJECT_COMPOSITE_PROPERTIES,
    QT_GALLERY_NFO_VISUAL_COMPOSITE_PROPERTIES
};

///////////
// Document
///////////

//nfo:Document : nfo:FileDataObject

//nfo:TextDocument : nfo:Document
//  nfo:characterCount, nfo:lineCount, nfo:wordCount

//nfo:PaginatedTextDocument : nfo:PaginatedTextDocument
//  nfo:pageCount

static const QGalleryItemProperty qt_galleryDocumentPropertyList[] =
{
    QT_GALLERY_NFO_FILEDATAOBJECT_PROPERTIES,
    QT_GALLERY_ITEM_PROPERTY("created"  , "nie:contentCreated(?x)", DateTime, CanRead | CanSort | CanFilter), \
    QT_GALLERY_ITEM_PROPERTY("pageCount", "nfo:pageCount(?x)"     , Int     , CanRead | CanSort | CanFilter),
    QT_GALLERY_ITEM_PROPERTY("wordCount", "nfo:wordCount(?x)"     , Int     , CanRead | CanSort | CanFilter)
};

static const QGalleryCompositeProperty qt_galleryDocumentCompositePropertyList[] =
{
    QT_GALLERY_NFO_FILEDATAOBJECT_COMPOSITE_PROPERTIES
};

///////
// Text
///////

//nfo:TextDocument : nfo:Document
//  nfo:characterCount, nfo:lineCount, nfo:wordCount

static const QGalleryItemProperty qt_galleryTextPropertyList[] =
{
    QT_GALLERY_NFO_FILEDATAOBJECT_PROPERTIES,
    QT_GALLERY_ITEM_PROPERTY("wordCount", "nfo:wordCount(?x)", Int, CanRead | CanSort | CanFilter)
};

static const QGalleryCompositeProperty qt_galleryTextCompositePropertyList[] =
{
    QT_GALLERY_NFO_FILEDATAOBJECT_COMPOSITE_PROPERTIES
};

////////
// Album
////////

//nmm:MusicAlbum : nfo:MediaList
//  nmm:albumPeakGain, nmm:albumGain, nmm:albumDuration, nmm:albumTitle, nmm:albumTrackCount
//  nmm:albumArtist,

static const QGalleryItemProperty qt_galleryAlbumPropertyList[] =
{
    QT_GALLERY_ITEM_PROPERTY("albumTitle" , "nmm:albumTitle(?x)"     , String, CanRead | CanWrite | CanFilter | CanSort),
    QT_GALLERY_ITEM_PROPERTY("title"      , "nmm:albumTitle(?x)"     , String, CanRead | CanWrite | CanFilter | CanSort),
    QT_GALLERY_ITEM_PROPERTY("trackCount" , "nmm:albumTrackCount(?x)", Int   , CanRead | CanSort | CanFilter),
    QT_GALLERY_LINKED_PROPERTY("artist"     , "nmm:artistName(?albumArtist)", " . ?x nmm:albumArtist ?albumArtist", String, CanRead | CanFilter | CanSort),
    QT_GALLERY_LINKED_PROPERTY("albumArtist", "nmm:artistName(?albumArtist)", " . ?x nmm:albumArtist ?albumArtist", String, CanRead | CanFilter | CanSort),
    QT_GALLERY_LINKED_PROPERTY("duration"   , "SUM(nfo:duration(?track))"   , " . ?track nmm:musicAlbum ?x"       , Int   , CanRead | CanSort | CanFilter),
};

/////////
// Artist
/////////

//nmm:Artist : nie:InformationElement
//  nmm:artistName

static const QGalleryItemProperty qt_galleryArtistPropertyList[] =
{
    QT_GALLERY_ITEM_PROPERTY("artist"    , "nmm:artistName(?x)"      , String, CanRead | CanWrite | CanFilter | CanSort),
    QT_GALLERY_ITEM_PROPERTY("title"     , "nmm:artistName(?x)"      , String, CanRead | CanWrite | CanFilter | CanSort),
    QT_GALLERY_ITEM_PROPERTY("duration"  , "SUM(nfo:duration(?track))", Int   , CanRead | CanSort | CanFilter),
    QT_GALLERY_ITEM_PROPERTY("trackCount", "COUNT(?track)"            , Int   , CanRead | CanSort | CanFilter),
};

/////////
// AlbumArtist
/////////

//nmm:Artist : nie:InformationElement
//  nmm:artistName

static const QGalleryItemProperty qt_galleryAlbumArtistPropertyList[] =
{
    QT_GALLERY_ITEM_PROPERTY("artist"    , "nmm:artistName(?x)"          , String, CanRead | CanWrite | CanFilter | CanSort),
    QT_GALLERY_ITEM_PROPERTY("title"     , "nmm:artistName(?x)"          , String, CanRead | CanWrite | CanFilter | CanSort),
    QT_GALLERY_LINKED_PROPERTY("duration"  , "SUM(nfo:duration(?track))", " . ?track nmm:musicAlbum ?album", Int, CanRead | CanSort | CanFilter),
    QT_GALLERY_LINKED_PROPERTY("trackCount", "COUNT(?track)"            , " . ?track nmm:musicAlbum ?album", Int, CanRead | CanSort | CanFilter),
};

/////////////
// PhotoAlbum
/////////////

//nfo:MediaList : nie:InformationElement
//  nfo:mediaListEntry, nfo:listDuration, nfo:entryCounter, nfo:hasMediaFileListEntry

//nmm:ImageList : nfo:MediaList

static const QGalleryItemProperty qt_galleryPhotoAlbumPropertyList[] =
{
    QT_GALLERY_ITEM_PROPERTY("count" , "nfo:entryCounter(?x)", Int   , CanRead | CanSort | CanFilter),
    QT_GALLERY_ITEM_PROPERTY("title" , "nie:title(?x)"       , String, CanRead | CanWrite | CanSort | CanFilter)
};

//////////////
// Audio Genre
//////////////

static const QGalleryItemProperty qt_galleryAudioGenrePropertyList[] =
{
    QT_GALLERY_ITEM_PROPERTY("duration"  , "SUM(nfo:duration(?x))", Int   , CanRead | CanFilter | CanSort),
    QT_GALLERY_ITEM_PROPERTY("genre"     , "nfo:genre(?x)"        , String, CanRead | CanFilter | CanSort),
    QT_GALLERY_ITEM_PROPERTY("title"     , "nfo:genre(?x)"        , String, CanRead | CanFilter | CanSort),
    QT_GALLERY_ITEM_PROPERTY("trackCount", "COUNT(?x)"            , Int   , CanRead | CanFilter | CanSort)

};

/////////////
// File Types
/////////////

static const QGalleryItemType qt_galleryItemTypeList[] =
{
    QT_GALLERY_ITEM_TYPE(File      , nfo, FileDataObject   , file      , File),
    QT_GALLERY_ITEM_TYPE(Folder    , nfo, Folder           , folder    , File),
    QT_GALLERY_ITEM_TYPE(Document  , nfo, Document         , document  , Document),
    QT_GALLERY_ITEM_TYPE(Audio     , nmm, MusicPiece       , audio     , Audio),
    QT_GALLERY_ITEM_TYPE(Image     , nmm, Photo            , image     , Image),
    QT_GALLERY_ITEM_TYPE(Video     , nmm, Video            , video     , Video),
    QT_GALLERY_ITEM_TYPE(Playlist  , nmm, Playlist         , playlist  , Playlist),
    QT_GALLERY_ITEM_TYPE(Text      , nfo, PlainTextDocument, text      , Text),
    QT_GALLERY_ITEM_TYPE_NO_COMPOSITE_FILTERED(Artist     , nmm, Artist    , " . ?track a nmm:MusicPiece . ?track nmm:performer ?x . ?track tracker:available true"  , artist     , Artist),
    QT_GALLERY_ITEM_TYPE_NO_COMPOSITE_FILTERED(AlbumArtist, nmm, Artist    , " . ?album a nmm:MusicAlbum . ?album nmm:albumArtist ?x . ?track a nmm:MusicPiece . ?track nmm:musicAlbum ?album . ?track tracker:available true", albumArtist, AlbumArtist),
    QT_GALLERY_ITEM_TYPE_NO_COMPOSITE_FILTERED(Album      , nmm, MusicAlbum, " . ?track a nmm:MusicPiece . ?track nmm:musicAlbum ?x . ?track tracker:available true", album     , Album),
    QT_GALLERY_ITEM_TYPE_NO_COMPOSITE(PhotoAlbum, nmm, ImageList , photoAlbum, PhotoAlbum),
    QT_GALLERY_AGGREGATE_TYPE_NO_COMPOSITE(AudioGenre, nmm, MusicPiece, nfo:genre(?x), audioGenre, AudioGenre),
};

class QGalleryTrackerServicePrefixColumn : public QGalleryTrackerCompositeColumn
{
public:
    QGalleryTrackerServicePrefixColumn() {}

    QVariant value(QVector<QVariant>::const_iterator row) const;
};

class QGalleryTrackerServiceTypeColumn : public QGalleryTrackerCompositeColumn
{
public:
    QGalleryTrackerServiceTypeColumn() {}

    QVariant value(QVector<QVariant>::const_iterator row) const;
};

class QGalleryTrackerServiceIndexColumn : public QGalleryTrackerValueColumn
{
public:
    QGalleryTrackerServiceIndexColumn() {}

    QVariant toVariant(TrackerSparqlCursor *cursor, int index) const;
};

QVariant QGalleryTrackerServicePrefixColumn::value(QVector<QVariant>::const_iterator row) const
{
    QGalleryItemTypeList itemTypes(qt_galleryItemTypeList);

    const int index = (row + 2)->toInt();

    return index != -1
            ? QVariant(QString(itemTypes[index].prefix) + row->toString())
            : QVariant(QLatin1String("file::") + row->toString());
}

QVariant QGalleryTrackerServiceTypeColumn::value(QVector<QVariant>::const_iterator row) const
{
    QGalleryItemTypeList itemTypes(qt_galleryItemTypeList);

    const int index = (row + 2)->toInt();

    return index != -1
            ? QVariant(itemTypes[index].itemType)
            : QVariant(QLatin1String("File"));
}

QVariant QGalleryTrackerServiceIndexColumn::toVariant(TrackerSparqlCursor *cursor, int index) const
{
    QGalleryItemTypeList itemTypes(qt_galleryItemTypeList);

    return itemTypes.indexOfRdfTypes(QString::fromUtf8(
                tracker_sparql_cursor_get_string(cursor, index, 0)).split(QLatin1Char(',')));
}

QGalleryTrackerSchema::QGalleryTrackerSchema(const QString &itemType)
    : m_itemIndex(QGalleryItemTypeList(qt_galleryItemTypeList).indexOfType(itemType))
{
}

QGalleryTrackerSchema::~QGalleryTrackerSchema()
{
}

QGalleryTrackerSchema QGalleryTrackerSchema::fromItemId(const QString &itemId)
{
    return QGalleryTrackerSchema(
            QGalleryItemTypeList(qt_galleryItemTypeList).indexOfItemId(itemId));
}

QString QGalleryTrackerSchema::itemType() const
{
    return m_itemIndex >= 0
            ? qt_galleryItemTypeList[m_itemIndex].itemType
            : QString();
}

int QGalleryTrackerSchema::serviceUpdateId(const QString &service)
{
    QGalleryItemTypeList itemTypes(qt_galleryItemTypeList);

    int index = itemTypes.indexOfService(service);

    return index != -1 ? itemTypes[index].updateId : FileId;
}

QStringList QGalleryTrackerSchema::supportedPropertyNames() const
{
    QStringList propertyNames;

    if (m_itemIndex >= 0) {
        const QGalleryItemType &type = qt_galleryItemTypeList[m_itemIndex];

        for (int i = 0; i < type.itemProperties.count; ++i)
            propertyNames.append(type.itemProperties[i].name);

        for (int i = 0; i < type.compositeProperties.count; ++i)
            propertyNames.append(type.compositeProperties[i].name);
    }
    return propertyNames;
}

QGalleryProperty::Attributes QGalleryTrackerSchema::propertyAttributes(
        const QString &propertyName) const
{
    if (m_itemIndex >= 0) {
        const QGalleryItemType &type = qt_galleryItemTypeList[m_itemIndex];

        int index;
        if ((index = type.itemProperties.indexOfProperty(propertyName)) != -1) {
            return type.itemProperties[index].attributes & PropertyMask;
        } else if ((index = type.compositeProperties.indexOfProperty(propertyName)) != -1) {
            QGalleryProperty::Attributes attributes = QGalleryProperty::CanRead;
            if (type.compositeProperties[index].writeFilterCondition)
                attributes |= QGalleryProperty::CanFilter;
            return attributes;
        }
    }
    return QGalleryProperty::Attributes();
}

QDocumentGallery::Error QGalleryTrackerSchema::prepareItemResponse(
        QGalleryTrackerResultSetArguments *arguments,
        const QString &itemId,
        const QStringList &propertyNames) const
{
    if (m_itemIndex >= 0) {
        QString query
                = QLatin1String(" FILTER(?x=<")
                + qt_galleryItemTypeList[m_itemIndex].prefix.strip(itemId).toString()
                + QLatin1String(">)");
        populateItemArguments(arguments, query, QString(), QString(), propertyNames, QStringList(), 0, 0);

        return QDocumentGallery::NoError;
    }

    return QDocumentGallery::ItemIdError;
}

QDocumentGallery::Error QGalleryTrackerSchema::prepareQueryResponse(
        QGalleryTrackerResultSetArguments *arguments,
        QGalleryQueryRequest::Scope scope,
        const QString &rootItemId,
        const QGalleryFilter &filter,
        const QStringList &propertyNames,
        const QStringList &sortPropertyNames,
        int offset,
        int limit) const
{
    if (m_itemIndex < 0) {
        return QDocumentGallery::ItemTypeError;
    } else {
        QString query;
        QString join;
        QString optionalJoin;

        QDocumentGallery::Error error = buildFilterQuery(&query, &join, &optionalJoin, scope, rootItemId, filter);

        if (error != QDocumentGallery::NoError) {
            return error;
        } else {
            populateItemArguments(
                    arguments, query, join, optionalJoin, propertyNames, sortPropertyNames, offset, limit);

            return QDocumentGallery::NoError;
        }
    }
}

QDocumentGallery::Error QGalleryTrackerSchema::prepareTypeResponse(
        QGalleryTrackerResultSetArguments *arguments) const
{
    if (m_itemIndex < 0)
        return QDocumentGallery::ItemTypeError;

    arguments->valueOffset = 1; // identity
    arguments->idColumn.reset(new QGalleryTrackerStaticColumn(QVariant()));
    arguments->urlColumn.reset(new QGalleryTrackerStaticColumn(QVariant()));
    arguments->typeColumn.reset(
            new QGalleryTrackerStaticColumn(qt_galleryItemTypeList[m_itemIndex].itemType));
    arguments->valueColumns = QVector<QGalleryTrackerValueColumn *>()
            << new QGalleryTrackerStringColumn
            << new QGalleryTrackerIntegerColumn;

    arguments->service = qt_galleryItemTypeList[m_itemIndex].service;
    arguments->updateMask = qt_galleryItemTypeList[m_itemIndex].updateMask;
    arguments->identityWidth = 1;
    arguments->tableWidth =  2;
    arguments->compositeOffset = 2;
    arguments->propertyNames << QStringLiteral("count");
    arguments->propertyAttributes << QGalleryProperty::CanRead;
    arguments->propertyTypes << QVariant::Int;

    if (qt_galleryItemTypeList[m_itemIndex].filterFragment) {
        arguments->sparql
                = QLatin1String("SELECT 'identity' COUNT(DISTINCT ")
                + qt_galleryItemTypeList[m_itemIndex].identity
                + QLatin1String(") WHERE {")
                + qt_galleryItemTypeList[m_itemIndex].typeFragment
                + QLatin1String(" FILTER(")
                + QLatin1String(qt_galleryItemTypeList[m_itemIndex].filterFragment)
                + QLatin1String(")}");
    } else {
        arguments->sparql
                = QLatin1String("SELECT 'identity' COUNT(DISTINCT ")
                + qt_galleryItemTypeList[m_itemIndex].identity
                + QLatin1String(") WHERE {")
                + qt_galleryItemTypeList[m_itemIndex].typeFragment
                + QLatin1String("}");
    }

    return QDocumentGallery::NoError;
}

QDocumentGallery::Error QGalleryTrackerSchema::buildFilterQuery(
        QString *query,
        QString *join,
        QString *optionalJoin,
        QGalleryQueryRequest::Scope scope,
        const QString &rootItemId,
        const QGalleryFilter &filter) const
{
    const QGalleryItemTypeList itemTypes(qt_galleryItemTypeList);

    QDocumentGallery::Error result = QDocumentGallery::NoError;

    QString filterStatement;

    if (!rootItemId.isEmpty()) {
        const int index = itemTypes.indexOfItemId(rootItemId);
        if (index != -1) {
            if (itemTypes[index].itemType == QDocumentGallery::Artist) {
                if (qt_galleryItemTypeList[m_itemIndex].itemType == QDocumentGallery::Album) {
                    *join   = QLatin1String(" . ?track nmm:performer <")
                            + itemTypes[index].prefix.strip(rootItemId).toString()
                            + QLatin1String(">");
                } else if (qt_galleryItemTypeList[m_itemIndex].itemType == QDocumentGallery::Audio) {
                    *join   = QLatin1String(" . ?x nmm:performer <")
                            + itemTypes[index].prefix.strip(rootItemId).toString()
                            + QLatin1String(">");
                } else {
                    result = QDocumentGallery::ItemIdError;
                }
            } else if (itemTypes[index].itemType == QDocumentGallery::AlbumArtist) {
                if (qt_galleryItemTypeList[m_itemIndex].itemType == QDocumentGallery::Audio) {
                    *join   = QLatin1String(" . ?album a nmm:MusicAlbum . ?x nmm:musicAlbum ?album . ?album nmm:albumArtist <")
                            + itemTypes[index].prefix.strip(rootItemId).toString()
                            + QLatin1String(">");
                } else if (qt_galleryItemTypeList[m_itemIndex].itemType == QDocumentGallery::Album) {
                    *join   = QLatin1String(" . ?x nmm:albumArtist <")
                            + itemTypes[index].prefix.strip(rootItemId).toString()
                            + QLatin1String(">");
                } else {
                    result = QDocumentGallery::ItemIdError;
                }
            } else if (itemTypes[index].itemType == QDocumentGallery::Folder) {
                const QString rootUrn = itemTypes[index].prefix.strip(rootItemId).toString();
                if (qt_galleryItemTypeList[m_itemIndex].updateMask & FileMask) {
                    if (scope == QGalleryQueryRequest::DirectDescendants) {
                        *join   = QLatin1String(" . ?x nfo:belongsToContainer <")
                                + itemTypes[index].prefix.strip(rootItemId).toString()
                                + QLatin1String(">");
                    } else {
                        filterStatement
                                = QLatin1String("tracker:uri-is-descendant(nie:url(<")
                                + rootUrn
                                + QLatin1String(">), nie:url(?x))");
                    }
                } else {
                    result = QDocumentGallery::ItemIdError;
                }
            } else if (itemTypes[index].itemType == QDocumentGallery::Album) {
                if (qt_galleryItemTypeList[m_itemIndex].itemType == QDocumentGallery::Audio) {
                    *join   = QLatin1String(" . ?x nmm:musicAlbum <")
                            + itemTypes[index].prefix.strip(rootItemId).toString()
                            + QLatin1String(">");
                } else {
                    result = QDocumentGallery::ItemIdError;
                }
            } else if (itemTypes[index].itemType == QDocumentGallery::PhotoAlbum
                       || itemTypes[index].itemType == QDocumentGallery::Playlist) {
                if ((qt_galleryItemTypeList[m_itemIndex].itemType == QDocumentGallery::Image
                        && itemTypes[index].itemType == QDocumentGallery::PhotoAlbum)
                        || (qt_galleryItemTypeList[m_itemIndex].itemType == QDocumentGallery::Audio
                        && itemTypes[index].itemType == QDocumentGallery::Playlist)) {
                    *join   = QLatin1String(" . <")
                            + itemTypes[index].prefix.strip(rootItemId).toString()
                            + QLatin1String("> nfo:hasMediaFileListEntry ?entry"
                                            " . ?entry nfo:entryUrl ?entryUrl"
                                            " . ?x nie:url ?entryUrl");
                } else {
                    result = QDocumentGallery::ItemIdError;
                }
            } else if (itemTypes[index].itemType == QDocumentGallery::AudioGenre) {
                if (qt_galleryItemTypeList[m_itemIndex].itemType == QDocumentGallery::Audio) {
                    *join   = QLatin1String(" . ?x nfo:genre '")
                            + itemTypes[index].prefix.strip(rootItemId).toString()
                            + QLatin1String("'");
                } else if (qt_galleryItemTypeList[m_itemIndex].itemType == QDocumentGallery::Album) {
                    *join   = QLatin1String(" . ?track nfo:genre '")
                            + itemTypes[index].prefix.strip(rootItemId).toString()
                            + QLatin1String("'");
                } else if (qt_galleryItemTypeList[m_itemIndex].itemType == QDocumentGallery::Artist) {
                    *join   = QLatin1String(" . ?track nfo:genre '")
                            + itemTypes[index].prefix.strip(rootItemId).toString()
                            + QLatin1String("'");
                } else if (qt_galleryItemTypeList[m_itemIndex].itemType == QDocumentGallery::AlbumArtist) {
                    *join   = QLatin1String(" . ?track nfo:genre '")
                            + itemTypes[index].prefix.strip(rootItemId).toString()
                            + QLatin1String("'");
                } else {
                    result = QDocumentGallery::ItemIdError;
                }
            } else {
                result = QDocumentGallery::ItemIdError;
            }
        } else {
            result = QDocumentGallery::ItemIdError;
        }
    }

    if (itemTypes[m_itemIndex].filterFragment) {
        if (!filterStatement.isEmpty()) {
            filterStatement
                    = QLatin1String(itemTypes[m_itemIndex].filterFragment)
                    + QLatin1String(" && ")
                    + filterStatement;
        } else {
            filterStatement = QLatin1String(itemTypes[m_itemIndex].filterFragment);
        }
    }

    if (filter.isValid()) {
        if (!filterStatement.isEmpty())
            filterStatement += QLatin1String(" && ");
        qt_writeCondition(
                &result,
                &filterStatement,
                optionalJoin,
                *join,
                filter,
                itemTypes[m_itemIndex].itemProperties,
                itemTypes[m_itemIndex].compositeProperties);
    }
    if (result == QDocumentGallery::NoError && !filterStatement.isEmpty()) {
        *query = QLatin1String(" FILTER(") + filterStatement + QLatin1String(")");
    }
    return result;
}

static QVector<QGalleryTrackerValueColumn *> qt_createValueColumns(
        const QVector<QVariant::Type> &types)
{
    QVector<QGalleryTrackerValueColumn *> columns;

    columns.reserve(types.count());

    for (int i = 0, count = types.count(); i < count; ++i) {
        switch (types.at(i)) {
        case QVariant::String:
            columns.append(new QGalleryTrackerStringColumn);
            break;
        case QVariant::StringList:
            columns.append(new QGalleryTrackerStringListColumn);
            break;
        case QVariant::Int:
            columns.append(new QGalleryTrackerIntegerColumn);
            break;
        case QVariant::Double:
            columns.append(new QGalleryTrackerDoubleColumn);
            break;
        case QVariant::DateTime:
            columns.append(new QGalleryTrackerDateTimeColumn);
            break;
        case QVariant::Url:
            columns.append(new QGalleryTrackerUrlColumn);
            break;
        default:
            Q_ASSERT(false);
            break;
        }
    }

    return columns;
}

static QString qt_writeSorting(
        QString *optionalJoin, const QString &join, const QStringList &propertyNames, const QGalleryItemPropertyList &properties)
{
    QString sortExpression;

    for (QStringList::const_iterator it = propertyNames.constBegin();
            it != propertyNames.constEnd();
            ++it) {
        if (it->startsWith(QLatin1Char('-'))) {
            const int propertyIndex = properties.indexOfProperty(it->mid(1));

            if (propertyIndex != -1) {
                const QGalleryItemProperty &property = properties[propertyIndex];
                sortExpression
                        += QLatin1String(" DESC(")
                        + property.field
                        + QLatin1String(")");

                if (property.join != QLatin1String(""))
                    qt_appendJoin(optionalJoin, join, property.join);
            }
        } else {
            const int propertyIndex = it->startsWith(QLatin1Char('+'))
                    ? properties.indexOfProperty(it->mid(1))
                    : properties.indexOfProperty(*it);

            if (propertyIndex != -1) {
                const QGalleryItemProperty &property = properties[propertyIndex];
                sortExpression
                        += QLatin1String(" ASC(")
                        + property.field
                        + QLatin1String(")");


                if (property.join != QLatin1String(""))
                    qt_appendJoin(optionalJoin, join, property.join);
            }
        }
    }

    return !sortExpression.isEmpty()
            ? QLatin1String(" ORDER BY") + sortExpression
            : sortExpression;
}

void QGalleryTrackerSchema::populateItemArguments(
        QGalleryTrackerResultSetArguments *arguments,
        const QString &query,
        const QString &join,
        const QString &optionalJoin,
        const QStringList &propertyNames,
        const QStringList &sortPropertyNames,
        int offset,
        int limit) const
{
    QString completeJoin = optionalJoin;
    QStringList fieldNames;
    QStringList valueNames;
    QStringList aliasNames;
    QStringList compositeNames;
    QVector<QGalleryProperty::Attributes> valueAttributes;
    QVector<QGalleryProperty::Attributes> aliasAttributes;
    QVector<QGalleryProperty::Attributes> compositeAttributes;
    QVector<QVariant::Type> valueTypes;
    QVector<QVariant::Type> extendedValueTypes;
    QVector<QVariant::Type> aliasTypes;
    QVector<QVariant::Type> compositeTypes;

    const QGalleryItemPropertyList &itemProperties
            = qt_galleryItemTypeList[m_itemIndex].itemProperties;
    const QGalleryCompositePropertyList &compositeProperties
            = qt_galleryItemTypeList[m_itemIndex].compositeProperties;

    for (QStringList::const_iterator it = propertyNames.begin(); it != propertyNames.end(); ++it) {
        if (valueNames.contains(*it) || aliasNames.contains(*it))
            continue;

        int propertyIndex;

        if ((propertyIndex = itemProperties.indexOfProperty(*it)) >= 0) {
            const QString field = itemProperties[propertyIndex].field;

            int fieldIndex = arguments->fieldNames.indexOf(field);

            if (fieldIndex >= 0) {
                arguments->aliasColumns.append(fieldIndex);
                aliasNames.append(*it);
                aliasAttributes.append(itemProperties[propertyIndex].attributes);
                aliasTypes.append(itemProperties[propertyIndex].type);
            } else {
                arguments->fieldNames.append(field);
                valueNames.append(*it);
                valueAttributes.append(itemProperties[propertyIndex].attributes);
                valueTypes.append(itemProperties[propertyIndex].type);

                if (itemProperties[propertyIndex].join != QLatin1String(""))
                    qt_appendJoin(&completeJoin, join, itemProperties[propertyIndex].join);
            }
        }
    }

    for (QStringList::const_iterator it = propertyNames.begin(); it != propertyNames.end(); ++it) {
        if (valueNames.contains(*it) || aliasNames.contains(*it) || compositeNames.contains(*it))
            continue;

        int propertyIndex;
        if ((propertyIndex = compositeProperties.indexOfProperty(*it)) >= 0) {
            const QGalleryItemPropertyList &dependencies
                    = compositeProperties[propertyIndex].dependencies;

            QVector<int> columns;
            for (int i = 0; i < dependencies.count; ++i) {
                const QString field = dependencies[i].field;

                int fieldIndex = arguments->fieldNames.indexOf(field);

                if (fieldIndex >= 0) {
                    columns.append(fieldIndex + 3);
                } else {
                    columns.append(arguments->fieldNames.count() + 3);

                    arguments->fieldNames.append(field);
                    extendedValueTypes.append(dependencies[i].type);
                }
            }

            QGalleryProperty::Attributes attributes = QGalleryProperty::CanRead;
            if (compositeProperties[propertyIndex].writeFilterCondition)
                attributes |= QGalleryProperty::CanFilter;

            compositeNames.append(*it);
            compositeAttributes.append(attributes);
            compositeTypes.append(compositeProperties[propertyIndex].type);
            arguments->compositeColumns.append(
                    compositeProperties[propertyIndex].createColumn(columns));
        }
    }

    if (qt_galleryItemTypeList[m_itemIndex].updateId & FileMask) {
        fieldNames = QStringList()
                     << qt_galleryItemTypeList[m_itemIndex].identity
                     << QLatin1String("nie:url(?x)")
                     << QLatin1String("rdf:type(?x)")
                     << arguments->fieldNames;
        arguments->valueOffset = 3;  // identity + nie:url + rdf:type
        arguments->idColumn.reset(new QGalleryTrackerServicePrefixColumn);
        arguments->urlColumn.reset(
                new QGalleryTrackerFileUrlColumn(QGALLERYTRACKERFILEURLCOLUMN_DEFAULT_COL));
        arguments->typeColumn.reset(new QGalleryTrackerServiceTypeColumn);
        arguments->valueColumns = QVector<QGalleryTrackerValueColumn *>()
                << new QGalleryTrackerStringColumn
                << new QGalleryTrackerUrlColumn
                << new QGalleryTrackerServiceIndexColumn
                << qt_createValueColumns(valueTypes + extendedValueTypes);
    } else {
        fieldNames = QStringList()
                     << qt_galleryItemTypeList[m_itemIndex].identity
                     << arguments->fieldNames;
        arguments->valueOffset = 1; // identity
        arguments->idColumn.reset(
                new QGalleryTrackerPrefixColumn(0, qt_galleryItemTypeList[m_itemIndex].prefix));
        arguments->urlColumn.reset(new QGalleryTrackerStaticColumn(QVariant()));
        arguments->typeColumn.reset(
                new QGalleryTrackerStaticColumn(qt_galleryItemTypeList[m_itemIndex].itemType));
        arguments->valueColumns = QVector<QGalleryTrackerValueColumn *>()
                << new QGalleryTrackerStringColumn
                << qt_createValueColumns(valueTypes + extendedValueTypes);
    }

    const QString sortFragment = qt_writeSorting(&completeJoin, join, sortPropertyNames, itemProperties);

    if (!completeJoin.isEmpty())
        completeJoin = QStringLiteral(" OPTIONAL {") + completeJoin.mid(3) + QStringLiteral("}");

    arguments->service = qt_galleryItemTypeList[m_itemIndex].service;
    arguments->updateMask = qt_galleryItemTypeList[m_itemIndex].updateMask;
    arguments->identityWidth = 1;
    arguments->tableWidth =  arguments->valueOffset + arguments->fieldNames.count();
    arguments->compositeOffset = arguments->valueOffset + valueNames.count();
    arguments->sparql
            = QLatin1String("SELECT ")
            + fieldNames.join(QLatin1String(" "))
            + QLatin1String(" WHERE {")
            + qt_galleryItemTypeList[m_itemIndex].typeFragment
            + join
            + completeJoin
            + query
            + QLatin1String("}")
            + QLatin1String(" GROUP BY ")
            + qt_galleryItemTypeList[m_itemIndex].identity
            + sortFragment;

    if (offset > 0)
        arguments->sparql += QString::fromLatin1(" OFFSET %1").arg(offset);
    if (limit > 0)
        arguments->sparql += QString::fromLatin1(" LIMIT %1").arg(limit);

    arguments->propertyNames = valueNames + compositeNames + aliasNames;
    arguments->propertyAttributes = valueAttributes + compositeAttributes + aliasAttributes;
    arguments->propertyTypes = valueTypes + compositeTypes + aliasTypes;

    for (int i = 0; i < arguments->propertyAttributes.count(); ++i) {
        if (arguments->propertyAttributes.at(i) & IsResource)
            arguments->resourceKeys.append(i + arguments->valueOffset );
        arguments->propertyAttributes[i] &= PropertyMask;
    }

    for (int i = 0; i < arguments->fieldNames.count(); ++i) {
        if (arguments->fieldNames.at(i).endsWith(QStringLiteral("(?x)")))
            arguments->fieldNames[i].chop(4);
        else
            arguments->fieldNames[i] = QString();
    }
}

QString QGalleryTrackerSchema::serviceForType( const QString &galleryType )
{
    QGalleryTypeList<QGalleryItemType> typeList(qt_galleryItemTypeList);
    int index = typeList.indexOfType(galleryType);
    if (index != -1)
        return typeList[index].service;

    qWarning() << galleryType << " does not exists";
    return QString();
}

QT_END_NAMESPACE_DOCGALLERY
