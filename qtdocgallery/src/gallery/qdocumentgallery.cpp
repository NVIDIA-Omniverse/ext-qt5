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

#include "qdocumentgallery.h"
#include "qabstractgallery_p.h"

#include <QtCore/qstringlist.h>

QT_BEGIN_NAMESPACE_DOCGALLERY

/*!
    \class QDocumentGallery

    \ingroup gallery
    \ingroup gallery-galleries

    \inmodule QtDocGallery

    \brief The QDocumentGallery class provides access to a gallery of documents
    and media present on a device.

    \section2 Requests

    Requests are the interface through which queries and actions are executed
    against a QDocumentGallery.

    \list

    \li QGalleryQueryRequest can be used with QDocumentGallery to find and
    navigate items present on a device.

    \li QGalleryItemRequest provides an API to query information about a single
    item in a QDocumentGallery.

    \li QGalleryTypeRequest provides a query for information about an item type
    supported by QDocumentGallery.

    \endlist

    \section2 Item types

    The items present in a QDocumentGallery can be of both file type such as
    documents and media, or categorical type such as genres or albums.

    \section3 File types

    \list
    \li \l File
    \li \l Folder
    \li \l Document
    \li \l Text
    \li \l Audio
    \li \l Image
    \li \l Video
    \li \l Playlist
    \endlist

    \section3 Categorical types

    \list
    \li \l Artist
    \li \l AlbumArtist
    \li \l Album
    \li \l AudioGenre
    \li \l PhotoAlbum
    \endlist

    \section2 Properties

    The item meta-data stored in a QDocumentGallery is addressed through
    properties.

    The set of properties addressable differs between item type but some
    properties are shared by most types.

    \section3 Common item properties

    Currently for the Tracker back-end, only the \l title property is available
    for the categorical types.

    \list
    \li \l author
    \li \l comments
    \li \l copyright
    \li \l description
    \li \l keywords
    \li \l language
    \li \l rating
    \li \l subject
    \li \l title
    \li \l url
    \endlist

    \section3 Common type properties

    \list
    \li \l count
    \endlist
*/

/*!
    \enum QDocumentGallery::Error
    This enum defines errors in a Gallery.

    \value NoError No error.
    \value NoGallery No Gallery defined or found.
    \value NotSupported Gallery is not supported.
    \value ConnectionError Connection to gallery is broken.
    \value ItemIdError Id of a item is wrong.
    \value ItemTypeError Type of an item is wrong.
    \value FilterError Error in filter found.
*/

/*!
    \variable QDocumentGallery::File

    This type matches all files in the document gallery.

    Properties typically available from files include:

    \list
    \li \l fileExtension
    \li \l fileName
    \li \l filePath
    \li \l fileSize
    \li \l lastModified
    \li \l lastAccessed
    \li \l mimeType
    \li \l path
    \endlist
*/

Q_DEFINE_GALLERY_TYPE(QDocumentGallery, File)

/*!
    \variable QDocumentGallery::Folder

    This type matches all file-system folders in the document gallery.

    In addition to the properties derived from the \l File type folders may
    also provide:

*/

Q_DEFINE_GALLERY_TYPE(QDocumentGallery, Folder)

/*!
    \variable QDocumentGallery::Document

    This type matches all document files in the document gallery.

    In addition to the properties derived from the \l File type documents may
    also provide:

    \list
    \li \l created
    \li \l pageCount
    \li \l wordCount
    \endlist
*/

Q_DEFINE_GALLERY_TYPE(QDocumentGallery, Document)

/*!
    \variable QDocumentGallery::Text

    This type matches all plain text files, e.g. HTML files, in the document
    gallery.

    In addition to the properties derived from the \l File type text files may
    also provide:

    \list
    \li \l wordCount
    \endlist
*/

Q_DEFINE_GALLERY_TYPE(QDocumentGallery, Text)

/*!
    \variable QDocumentGallery::Audio

    This type matches all audio files in the document gallery.

    In addition to the properties derived from the \l File type audio files may
    also provide:

    \list
    \li \l albumArtist
    \li \l albumTitle
    \li \l artist
    \li \l audioBitRate
    \li \l audioCodec
    \li \l channelCount
    \li \l composer
    \li \l discNumber
    \li \l genre
    \li \l duration
    \li \l lastPlayed
    \li \l lyrics
    \li \l performer
    \li \l playCount
    \li \l producer
    \li \l sampleRate
    \li \l trackNumber
    \endlist
*/

Q_DEFINE_GALLERY_TYPE(QDocumentGallery, Audio)

/*!
    \variable QDocumentGallery::Image

    This type matches all image files in the document gallery.

    In addition to the properties derived from the \l File type image files may
    also provide:

    \list
    \li \l altitude
    \li \l cameraManufacturer
    \li \l cameraModel
    \li \l dateTaken
    \li \l exposureProgram
    \li \l exposureTime
    \li \l flashEnabled
    \li \l fNumber
    \li \l focalLength
    \li \l height
    \li \l longitude
    \li \l latitude
    \li \l meteringMode
    \li \l orientation
    \li \l width
    \li \l whiteBalance
    \endlist
*/

Q_DEFINE_GALLERY_TYPE(QDocumentGallery, Image)

/*!
    \variable QDocumentGallery::Video

    This type matches all video files in the document gallery.

    In addition to the properties derived from the \l File type video files may
    also provide:

    \list
    \li \l audioBitRate
    \li \l audioCodec
    \li \l channelCount
    \li \l director
    \li \l duration
    \li \l frameRate
    \li \l height
    \li \l lastPlayed
    \li \l performer
    \li \l playCount
    \li \l producer
    \li \l resumePosition
    \li \l sampleRate
    \li \l videoBitRate
    \li \l videoCodec
    \li \l width
    \endlist
*/

Q_DEFINE_GALLERY_TYPE(QDocumentGallery, Video)

/*!
    \variable QDocumentGallery::Playlist

    This type matches all playlist files in the document gallery.

    In addition to the properties derived from the \l File type playlist files
    may also provide:

    \list
    \li \l duration
    \li \l trackCount
    \endlist

*/

Q_DEFINE_GALLERY_TYPE(QDocumentGallery, Playlist)

/*!
    \variable QDocumentGallery::Artist

    This type matches all artists in the document gallery.

    Properties typically available from artists include:

    \list
    \li \l artist
    \li \l duration
    \li \l trackCount
    \endlist
*/

Q_DEFINE_GALLERY_TYPE(QDocumentGallery, Artist)

/*!
    \variable QDocumentGallery::AlbumArtist

    This type matches all album artists in the document gallery.

    Properties typically available from album artists include:

    \list
    \li \l albumArtist
    \li \l artist
    \li \l duration
    \li \l trackCount
    \endlist
*/

Q_DEFINE_GALLERY_TYPE(QDocumentGallery, AlbumArtist)

/*!
    \variable QDocumentGallery::Album

    This type matches all albums in the document gallery.

    Properties typically available from albums include:

    \list
    \li \l albumArtist
    \li \l albumTitle
    \li \l artist
    \li \l duration
    \li \l trackCount
    \endlist
*/

Q_DEFINE_GALLERY_TYPE(QDocumentGallery, Album)

/*!
    \variable QDocumentGallery::AudioGenre

    This type matches all audio genres in the document gallery.

    Properties typically available from genres include:

    \list
    \li \l duration
    \li \l genre
    \li \l trackCount
    \endlist
*/

Q_DEFINE_GALLERY_TYPE(QDocumentGallery, AudioGenre)

/*!
    \variable QDocumentGallery::PhotoAlbum

    This type matches all photo albums in the document gallery.

    \list
    \li \l count
    \endlist
*/

Q_DEFINE_GALLERY_TYPE(QDocumentGallery, PhotoAlbum)

// Common

/*!
    \variable QDocumentGallery::url

    This property contains canonical url of an item in the document gallery.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, url)

/*!
    \variable QDocumentGallery::author

    This property contains the name of the author of an item in the document
    gallery.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, author)

/*!
    \variable QDocumentGallery::copyright

    This property contains a copyright statement for an item in the document
    gallery.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, copyright)

/*!
    \variable QDocumentGallery::description

    This property contains a description of an item in the document gallery.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, description)

/*!
    \variable QDocumentGallery::comments

    This property contains a user comment about an item in the document gallery.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, comments)

/*!
    \variable QDocumentGallery::rating

    This property contains a rating for an item in the document gallery.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, rating)

/*!
    \variable QDocumentGallery::title

    This property contains the title of an item in the document gallery.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, title)

/*!
    \variable QDocumentGallery::subject

    This property contains the subject of item in the document gallery.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, subject)

/*!
    \variable QDocumentGallery::keywords

    This property contains keywords relevant to an item in the document gallery.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, keywords)

/*!
    \variable QDocumentGallery::language

    This property contains the language of the content of an item in the
    document gallery.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, language)

// File

/*!
    \variable QDocumentGallery::path

    This property contains the absolute path excluding the file name of a file
    in the document gallery.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, path)

/*!
    \variable QDocumentGallery::filePath

    This property contains the absolute path including the file name of a file
    in the document gallery.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, filePath)

/*!
    \variable QDocumentGallery::fileName

    This property contains the file name excluding the path of a file in
    the document gallery.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, fileName)

/*!
    \variable QDocumentGallery::fileExtension

    This property contains the file extension of a file.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, fileExtension)

/*!
    \variable QDocumentGallery::fileSize

    This property contains the size in bytes of a file.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, fileSize)

/*!
    \variable QDocumentGallery::mimeType

    This property contains the MIME type of a file.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, mimeType)

/*!
    \variable QDocumentGallery::lastModified

    This property contains the date and time a file was last modified.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, lastModified)

/*!
    \variable QDocumentGallery::lastAccessed

    This property contains the date and time a file was last accessed.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, lastAccessed)

// Document

/*!
    \variable QDocumentGallery::pageCount

    This property contains the number of pages in a document.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, pageCount)

/*!
    \variable QDocumentGallery::wordCount

    This property contains the number of words in document.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, wordCount)

/*!
    \variable QDocumentGallery::created

    This property contains the date and time a document was created.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, created)

// Media

/*!
    \variable QDocumentGallery::duration

    This property contains the duration of an audio or video file.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, duration)

/*!
    \variable QDocumentGallery::producer

    This property contains the name of the producer of a media file.

    Note that this property for \l Audio is not supported by Tracker.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, producer)

/*!
    \variable QDocumentGallery::lastPlayed

    This property contains the date and time an audio or video file was last
    played.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, lastPlayed)

/*!
    \variable QDocumentGallery::playCount

    This property contains the number of times an audio or video file has been
    played.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, playCount)

/*!
    \variable QDocumentGallery::performer

    This property contains the names of performers in a media file.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, performer)

// Audio

/*!
    \variable QDocumentGallery::audioCodec

    This property contains the name of the codec used to encode audio in a media
    file.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, audioCodec)

/*!
    \variable QDocumentGallery::audioBitRate

    This property contains the bit rate of the audio in a media file.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, audioBitRate)

/*!
    \variable QDocumentGallery::sampleRate

    This property contains the sample rate of the audio in a media file.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, sampleRate)
/*!
    \variable QDocumentGallery::channelCount

    This property contains the number of audio channels in a media file.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, channelCount)

// Music

/*!
    \variable QDocumentGallery::artist

    This property contains the names of artists contributing to a music track.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, artist)

/*!
    \variable QDocumentGallery::albumArtist

    This property contains the name of the title artist of a music album.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, albumArtist)

/*!
    \variable QDocumentGallery::albumTitle

    This property contains the title of the album a music track belongs to.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, albumTitle)

/*!
    \variable QDocumentGallery::composer

    This property contains the name of the composer of a music track.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, composer)

/*!
    \variable QDocumentGallery::genre

    This property contains the genre of a media file.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, genre)

/*!
    \variable QDocumentGallery::lyrics

    This property contains the lyrics to a music track.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, lyrics)

/*!
    \variable QDocumentGallery::trackCount

    This property contains the number of music tracks in an album.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, trackCount)

/*!
    \variable QDocumentGallery::trackNumber

    This property contains the track number of a music track in an album.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, trackNumber)

/*!
    \variable QDocumentGallery::discNumber

    This property contains the disc number of an album in a set.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, discNumber)

// Image, Video common.

/*!
    \variable QDocumentGallery::width

    This property contains the width in pixels of an image or video file.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, width)

/*!
    \variable QDocumentGallery::height

    This property contains the height in pixels of an image or video file.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, height)

// Image
/*!
    \variable QDocumentGallery::orientation

    This property contains the orientation of an image.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, orientation)

// Photo

/*!
    \variable QDocumentGallery::dateTaken

    This property contains the date and time a photo was taken.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, dateTaken)

/*!
    \variable QDocumentGallery::cameraManufacturer

    This property contains the manufacturer name of the camera used to take a
    photo.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, cameraManufacturer)

/*!
    \variable QDocumentGallery::cameraModel

    This property contains the model name of the camera used to take a photo.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, cameraModel)

/*!
    \variable QDocumentGallery::exposureProgram

    This property contains the name of the exposure program used when taking a
    photo.

    Note that this property is not supported by Tracker.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, exposureProgram)

/*!
    \variable QDocumentGallery::exposureTime

    This property contains the exposure time of a photo in seconds.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, exposureTime)

/*!
    \variable QDocumentGallery::fNumber

    This property contains the F-number of a photo.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, fNumber)

/*!
    \variable QDocumentGallery::flashEnabled

    This property contains whether a flash was used when taking a photo.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, flashEnabled)

/*!
    \variable QDocumentGallery::focalLength

    This property contains the focal length used when taking a photo.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, focalLength)

/*!
    \variable QDocumentGallery::meteringMode

    This property contains the metering mode used when taking a photo.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, meteringMode)

/*!
    \variable QDocumentGallery::whiteBalance

    This property contains the white balance setting used when taking a photo.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, whiteBalance)

/*!
    \variable QDocumentGallery::latitude

    This property contains the latitude of the location where the photo was captured.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, latitude)

/*!
    \variable QDocumentGallery::longitude

    This property contains the longitude of the location where the photo was captured.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, longitude)

/*!
    \variable QDocumentGallery::altitude

    This property contains the altitude of the location where the photo was captured.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, altitude)

// Video

/*!
    \variable QDocumentGallery::frameRate

    This property contains the frame rate of a video.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, frameRate)

/*!
    \variable QDocumentGallery::videoCodec

    This property contains the codec used to encode the video in a media file.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, videoCodec)

/*!
    \variable QDocumentGallery::videoBitRate

    This property contains the bit rate of the video in a media file.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, videoBitRate)

/*!
    \variable QDocumentGallery::resumePosition

    This property contains the position in a video where playback was
    interrupted.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, resumePosition)

/*!
    \variable QDocumentGallery::director

    This property contains the name of the director of a video.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, director)

// Type

/*!
    \variable QDocumentGallery::count

    This property contains the number of items within the scope of a parent
    item or an item type.
*/

Q_DEFINE_GALLERY_PROPERTY(QDocumentGallery, count)

/*!
    \fn QDocumentGallery::QDocumentGallery(QObject *parent)

    Constructs a new document gallery.

    The \a parent is passed to QAbstractGallery.
*/

/*!
    \fn QDocumentGallery::~QDocumentGallery()

    Destroys a document gallery.
*/

/*!
    \fn QDocumentGallery::isRequestSupported(QGalleryAbstractRequest::RequestType type) const

    \reimp
*/

/*!
    \fn QDocumentGallery::itemTypePropertyNames(const QString &itemType) const;

    Returns a list of names of properties that are valid for an \a itemType.
*/

/*!
    \fn QDocumentGallery::propertyAttributes(const QString &propertyName, const QString &itemType) const

    Returns the attributes of a property of \a itemType identified by
    \a propertyName
*/

/*!
    \fn QDocumentGallery::createResponse(QGalleryAbstractRequest *request)

    \reimp
*/


#ifdef QT_DOCUMENT_GALLERY_NULL

QDocumentGallery::QDocumentGallery(QObject *parent)
    : QAbstractGallery(parent)
{
}

QDocumentGallery::~QDocumentGallery()
{
}

bool QDocumentGallery::isRequestSupported(QGalleryAbstractRequest::RequestType) const
{
    return false;
}

QStringList QDocumentGallery::itemTypePropertyNames(const QString &) const
{
    return QStringList();
}

QGalleryProperty::Attributes QDocumentGallery::propertyAttributes(const QString &, const QString &) const
{
    return QGalleryProperty::Attributes();
}

QGalleryAbstractResponse *QDocumentGallery::createResponse(QGalleryAbstractRequest *)
{
    return 0;
}

#endif // QT_DOCUMENT_GALLERY_NULL

#include "moc_qdocumentgallery.cpp"

QT_END_NAMESPACE_DOCGALLERY
