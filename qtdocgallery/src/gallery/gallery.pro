TARGET = QtDocGallery
MODULE = docgallery
QT = core

load(qt_module)

PUBLIC_HEADERS += \
    qgalleryglobal.h \
    qabstractgallery.h \
    qdocumentgallery.h \
    qgalleryabstractrequest.h \
    qgalleryabstractresponse.h \
    qgalleryfilter.h \
    qgalleryitemrequest.h \
    qgalleryproperty.h \
    qgalleryquerymodel.h \
    qgalleryqueryrequest.h \
    qgalleryresource.h \
    qgalleryresultset.h \
    qgallerytype.h \
    qgallerytyperequest.h

PRIVATE_HEADERS += \
    qabstractgallery_p.h \
    qgalleryabstractrequest_p.h \
    qgalleryabstractresponse_p.h \
    qgallerynullresultset_p.h \
    qgalleryresultset_p.h

SOURCES += \
    qabstractgallery.cpp \
    qdocumentgallery.cpp \
    qgalleryabstractrequest.cpp \
    qgalleryabstractresponse.cpp \
    qgalleryfilter.cpp \
    qgalleryitemrequest.cpp \
    qgalleryquerymodel.cpp \
    qgalleryqueryrequest.cpp \
    qgalleryresource.cpp \
    qgalleryresultset.cpp \
    qgallerytyperequest.cpp

OTHER_FILES = \
    qgalleryproperty.qdoc \
    qgallerytype.qdoc

linux-* {
    qtHaveModule(dbus):contains(tracker_enabled, yes) {
        include (tracker/tracker.pri)
    } else: {
        DEFINES += QT_DOCUMENT_GALLERY_NULL
    }
} else {
    DEFINES += QT_DOCUMENT_GALLERY_NULL
}

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS
