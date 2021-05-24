INCLUDEPATH += $$PWD

QT += dbus

CONFIG += link_pkgconfig
PKGCONFIG_PRIVATE += tracker-sparql-0.14

PRIVATE_HEADERS += \
        $$PWD/qgallerydbusinterface_p.h \
        $$PWD/qgallerytrackerchangenotifier_p.h \
        $$PWD/qgallerytrackereditableresultset_p.h \
        $$PWD/qgallerytrackerlistcolumn_p.h \
        $$PWD/qgallerytrackermetadataedit_p.h \
        $$PWD/qgallerytrackerresultset_p.h \
        $$PWD/qgallerytrackerresultset_p_p.h \
        $$PWD/qgallerytrackerschema_p.h \
        $$PWD/../../3rdparty/synchronization/synchronizelists.h

SOURCES += \
        $$PWD/qgallerydbusinterface.cpp \
        $$PWD/qdocumentgallery_tracker.cpp \
        $$PWD/qgallerytrackerchangenotifier.cpp \
        $$PWD/qgallerytrackereditableresultset.cpp \
        $$PWD/qgallerytrackerlistcolumn.cpp \
        $$PWD/qgallerytrackermetadataedit.cpp \
        $$PWD/qgallerytrackerresultset.cpp \
        $$PWD/qgallerytrackerschema.cpp
