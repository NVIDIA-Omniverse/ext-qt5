TEMPLATE = subdirs
SUBDIRS += \
    cmake \
    qdocumentgallery \
    qgalleryabstractrequest \
    qgalleryabstractresponse \
    qgalleryfilter \
    qgalleryitemrequest \
    qgalleryquerymodel \
    qgalleryqueryrequest \
    qgalleryresource \
    qgallerytyperequest \
#    qdeclarativedocumentgalleryitem \
#    qdeclarativedocumentgallerymodel \
#    qdeclarativedocumentgallerytype

linux-*:qtHaveModule(dbus):contains(tracker_enabled, yes) {
    SUBDIRS += \
            qgallerytrackerschema_tracker
#        qgallerytrackerresultset_tracker \
}

