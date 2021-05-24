OTHER_FILES += \
               $$PWD/qtdocgallery.qdocconf \
               $$PWD/qtdocgallery-dita.qdocconf

QDOC = $$QT.core.bins/qdoc

ONLINE_CONF = $$PWD/qtdocgallery.qdocconf
DITA_CONF = $$PWD/qtdocgallery-dita.qdocconf
QCH_CONF = #nothing yet

equals(QMAKE_DIR_SEP, \\) {
    QDOC = $$replace(QDOC, "qdoc", "qdoc.exe")
    ONLINE_CONF = $$replace(ONLINE_CONF, "/", "\\")
    DITA_DOCS = $$replace(ONLINE_CONF, "/", "\\")
}

qtdocgallery_online_docs.commands = $$QDOC $$ONLINE_CONF
qtdocgallery_dita_docs.commands = $$QDOC $$DITA_CONF

qtdocgallery_docs.depends = qtdocgallery_dita_docs qtdocgallery_online_docs
QMAKE_EXTRA_TARGETS = qtdocgallery_docs qtdocgallery_dita_docs qtdocgallery_online_docs
QMAKE_CLEAN += \
               "-r $$PWD/html" \
               "-r $$PWD/ditaxml"
