include(../auto.pri)

SOURCES += tst_qdocumentgallery.cpp

linux-*: {
    qtHaveModule(dbus):contains(tracker_enabled, yes): DEFINES += QT_TRACKER_ENABLED
}
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
