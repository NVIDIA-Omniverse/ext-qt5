/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/


#include <QtCore/QtCore>
#include <QtTest/QtTest>

#include "global.h"

#ifdef QT_NAMESPACE
#define STRINGIFY_HELPER(s) #s
#define STRINGIFY(s) STRINGIFY_HELPER(s)
QString ns = STRINGIFY(QT_NAMESPACE) + QString("::");
#else
QString ns;
#endif

class tst_Symbols: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();

    void globalObjects();
#ifndef QT_CROSS_COMPILED
    void prefix();
#endif

private:
    QString qtModuleDir;
    QString qtLibDir;
    QHash<QString, QString> modules;
    QStringList keys;
};

void tst_Symbols::initTestCase()
{
    qtModuleDir = QString::fromLocal8Bit(qgetenv("QT_MODULE_TO_TEST"));
    if (qtModuleDir.isEmpty()) {
        QSKIP("$QT_MODULE_TO_TEST is unset - nothing to test.  Set QT_MODULE_TO_TEST to the path "
              "of a Qt module to test.");
    }

    QString configFile = qtModuleDir + "/tests/global/global.cfg";
    if (!QFile(configFile).exists()) {
        QSKIP(
            qPrintable(QString(
                "%1 does not exist.  Create it if you want to run this test."
            ).arg(configFile))
        );
    }

    QString workDir = qtModuleDir + QStringLiteral("/tests/global");
    modules = qt_tests_shared_global_get_modules(workDir, configFile);

    if (!modules.size())
        QSKIP("No modules found.");

    qtLibDir = QLibraryInfo::location( QLibraryInfo::LibrariesPath );
    QFileInfo qtLibDirInfo(qtLibDir);
    QVERIFY2(qtLibDirInfo.isDir(), qPrintable(
        QString("QLibraryInfo::LibrariesPath `%1' %2\nIs your build complete and installed?")
        .arg(qtLibDir)
        .arg(!qtLibDirInfo.exists() ? "doesn't exist" : "isn't a directory")
    ));

    keys = modules.keys();
    QList<QString>::iterator i;
    for (i = keys.begin(); i != keys.end(); ++i)
        *i = "lib" + *i + ".so";
    qDebug() << qtLibDir << keys;
}

/* Computes the line number from a symbol */
static QString symbolToLine(const QString &symbol, const QString &lib)
{
    // nm outputs the symbol name, the type, the address and the size
    QRegularExpression re("global constructors keyed to ([a-zA-Z_0-9.]*) (.) ([0-f]*) ([0-f]*)");
    QRegularExpressionMatch match = re.match(symbol);
    if (!match.hasMatch())
        return QString();

    // address and symbolSize are in hex. Convert to integers
    bool ok;
    int symbolAddress = match.captured(3).toInt(&ok, 16);
    if (!ok)
        return QString();
    int symbolSize = match.captured(4).toInt(&ok, 16);
    if (!ok)
        return QString();

    // now, find the start address, which is the address - size
    QString startAddress = QString::number(symbolAddress - symbolSize, 16);

    QProcess proc;
    proc.start("addr2line", QStringList() << "-e" << lib << startAddress);
    if (!proc.waitForFinished())
        return QString();

    QString result = QString::fromLocal8Bit(proc.readLine());
    result.chop(1); // chop tailing newline
    return result;
}

/* This test searches through all Qt libraries and searches for symbols
   starting with "global constructors keyed to "

   These indicate static global objects, which should not be used in shared
   libraries - use Q_GLOBAL_STATIC instead.
*/
void tst_Symbols::globalObjects()
{
    // these are regexps for global objects that are allowed in Qt
    QStringList whitelist = QStringList()
        // ignore qInitResources - they are safe to use
        << "^_Z[0-9]*qInitResources_"
        << "qrc_.*\\.cpp"
        // ignore qRegisterGuiVariant - it's a safe fallback to register GUI Variants
        << "qRegisterGuiVariant";

    bool isFailed = false;

    QDir dir(qtLibDir, "*.so");
    QStringList files = dir.entryList();
    QVERIFY(!files.isEmpty());

    foreach (QString lib, files) {
        if (!keys.contains(lib))
            continue;

        if (lib == "libQtCLucene.so") {
            // skip this library, it's 3rd-party C++
            continue;
        }

        qDebug() << lib
                 << ", " << dir.absolutePath();

        QProcess proc;
        proc.start("nm",
           QStringList() << "-C" << "--format=posix" << "--dynamic"
                         << dir.absolutePath() + "/" + lib);
        QVERIFY(proc.waitForFinished());
        QCOMPARE(proc.exitCode(), 0);
        QCOMPARE(QString::fromLocal8Bit(proc.readAllStandardError()), QString());

        QStringList symbols = QString::fromLocal8Bit(proc.readAll()).split("\n");
        QVERIFY(!symbols.isEmpty());
        foreach (QString symbol, symbols) {
            if (symbol.isEmpty())
                continue;

            if (!symbol.startsWith("global constructors keyed to "))
                continue;

            QRegularExpression re("global constructors keyed to ([a-zA-Z_0-9.]*)");
            QRegularExpressionMatch match = re.match(symbol);
            QVERIFY(match.hasMatch());

            QString cap = match.captured(1);

            bool whitelisted = false;
            foreach (QString white, whitelist) {
                if (cap.indexOf(QRegularExpression(white)) != -1) {
                    whitelisted = true;
                    break;
                }
            }
            if (whitelisted)
                continue;

            QString line = symbolToLine(symbol, dir.absolutePath() + "/" + lib);

            if (cap.contains('.'))
                QWARN(qPrintable("Static global object(s) found in " + lib + " in file " + cap + " (" + line + ")"));
            else
                QWARN(qPrintable("Static global object found in " + lib + " near symbol " + cap + " (" + line + ")"));

            isFailed = true;
        }
    }

    if (isFailed) {
        QVERIFY2(!isFailed, "Libraries contain static global objects. See Debug output above.");
    }
}

// This test needs a compiler on the target, which is unlikely if cross-compiled.
#ifndef QT_CROSS_COMPILED
void tst_Symbols::prefix()
{
    QStringList qtTypes;
    qtTypes << "QString" << "QChar" << "QWidget" << "QObject" << "QVariant" << "QList"
            << "QMap" << "QHash" << "QVector" << "QRect" << "QSize" << "QPoint"
            << "QTextFormat" << "QTextLength" << "QPen" << "QFont" << "QIcon"
            << "QPixmap" << "QImage" << "QRegion" << "QPolygon";
    QStringList qAlgorithmFunctions;
    qAlgorithmFunctions << "qBinaryFind" << "qLowerBound" << "qUpperBound"
                        << "qAbs" << "qMin" << "qMax" << "qBound" << "qSwap"
                        << "qHash" << "qDeleteAll" << "qCopy" << "qSort";

    QStringList exceptionalSymbols;
    exceptionalSymbols << "XRectangle::~XRectangle()"
                       << "XChar2b::~XChar2b()"
                       << "XPoint::~XPoint()"
                       << "glyph_metrics_t::"; // #### Qt 4.2

    QStringList stupidCSymbols;
    stupidCSymbols << "Add_Glyph_Property"
                   << "Check_Property"
                   << "Coverage_Index"
                   << "Get_Class"
                   << "Get_Device"
                   << "rcsid3"
                   << "sfnt_module_class"
                   << "t1cid_driver_class"
                   << "t42_driver_class"
                   << "winfnt_driver_class"
                   << "pshinter_module_class"
                   << "psnames_module_class"
                   // C symbols from Qt
                   << "qt_addObject"
                   << "qt_removeObject"
                   << "qt_startup_hook"
                   ;

    QHash<QString,QStringList> excusedPrefixes;
    excusedPrefixes[QString()] =
        QStringList() << "Ui_Q"; // uic generated, like Ui_QPrintDialog

    excusedPrefixes["QtCore"] =
        QStringList() << "hb_"
                      << "HB_"
                      // zlib symbols, for -qt-zlib ;(
                      << "deflate"
                      << "compress"
                      << "uncompress"
                      << "adler32"
                      << "gz"
                      << "inflate"
                      << "zlib"
                      << "zError"
                      << "get_crc_table"
                      << "crc32";

    excusedPrefixes["QtGui"] =
        QStringList() << "ftglue_"
                      << "Load_"
                      << "otl_"
                      << "TT_"
                      << "tt_"
                      << "t1_"
                      << "Free_"
                      << "FT_"
                      << "FTC_"
                      << "ft_"
                      << "ftc_"
                      << "af_autofitter"
                      << "af_dummy"
                      << "af_latin"
                      << "autofit_"
                      << "XPanorami"
                      << "Xinerama"
                      << "bdf_"
                      << "ccf_"
                      << "gray_raster"
                      << "pcf_"
                      << "cff_"
                      << "otv_"
                      << "pfr_"
                      << "ps_"
                      << "psaux"
                      << "png_";

    excusedPrefixes["QtSql"] =
        QStringList() << "sqlite3";

    excusedPrefixes["QtWebKit"] =
        QStringList() << "WebCore::"
                      << "KJS::"
                      << "kjs"
                      << "kJS"
                      << "JS"
//                      << "OpaqueJS"
                      << "WTF"
                      << "wtf_"
                      << "SVG::"
                      << "NPN_"
                      << "cti"  // ctiTrampoline and ctiVMThrowTrampoline from the JIT
#ifdef QT_NAMESPACE
                      << "QWeb" // Webkit is only 'namespace aware'
                      << "qWeb"
                      << "qt"
                      << "QGraphicsWebView"
                      << "operator"
#endif
        ;

    excusedPrefixes["phonon"] =
        QStringList() << ns + "Phonon";

    QDir dir(qtLibDir, "*.so");
    QStringList files = dir.entryList();
    QVERIFY(!files.isEmpty());

    bool isFailed = false;
    foreach (QString lib, files) {
        if (!keys.contains(lib))
            continue;

        if (lib.contains("Designer") || lib.contains("QtCLucene") || lib.contains("XmlPatternsSDK"))
            continue;

        bool isPhonon = lib.contains("phonon");

        qDebug() << lib
                 << ", " << dir.absolutePath();

        QProcess proc;
        proc.start("nm",
           QStringList() << "-g" << "-C" << "-D" << "--format=posix"
                         << "--defined-only" << dir.absolutePath() + "/" + lib);
        QVERIFY(proc.waitForFinished());
        QCOMPARE(proc.exitCode(), 0);
        QCOMPARE(QString::fromLocal8Bit(proc.readAllStandardError()), QString());

        QStringList symbols = QString::fromLocal8Bit(proc.readAll()).split("\n");
        QVERIFY(!symbols.isEmpty());
        foreach (QString symbol, symbols) {
            if (symbol.isEmpty())
                continue;

            if (symbol.startsWith("unsigned "))
                // strip modifiers
                symbol = symbol.mid(symbol.indexOf(' ') + 1);
            if (symbol.startsWith("long long ")) {
                symbol = symbol.mid(10);
            } else if (symbol.startsWith("bool ") || symbol.startsWith("bool* ")
                || symbol.startsWith("char ") || symbol.startsWith("char* ")
                || symbol.startsWith("int ") || symbol.startsWith("int* ") || symbol.startsWith("int*&")
                || symbol.startsWith("short") || symbol.startsWith("long ")
                || symbol.startsWith("void ") || symbol.startsWith("void* ")
                || symbol.startsWith("double ") || symbol.startsWith("double* ")
                || symbol.startsWith("float ") || symbol.startsWith("float* ")) {
                // partial templates have the return type in their demangled name, strip it
                symbol = symbol.mid(symbol.indexOf(' ') + 1);
            }
            if (symbol.startsWith("const ") || symbol.startsWith("const* ") ||
                symbol.startsWith("const& ")) {
                // strip modifiers
                symbol = symbol.mid(symbol.indexOf(' ') + 1);
            }

            if (symbol.mid(symbol.indexOf(' ')+1).startsWith("std::"))
                continue;
            if (symbol.startsWith("_") || symbol.startsWith("std::"))
                continue;
            if (symbol.startsWith("vtable ") || symbol.startsWith("VTT for ") ||
                symbol.startsWith("construction vtable for"))
                continue;
            if (symbol.startsWith("typeinfo "))
                continue;
            if (symbol.startsWith("non-virtual thunk ") || symbol.startsWith("virtual thunk"))
                continue;
            if (symbol.startsWith(ns + "operator"))
                continue;
            if (symbol.startsWith("operator new") || symbol.startsWith("operator delete"))
                continue;
            if (symbol.startsWith("guard variable for "))
                continue;
            if (symbol.contains("(" + ns + "QTextStream"))
                // QTextStream is excused.
                continue;
            if (symbol.contains("(" + ns + "Q3TextStream"))
                // Q3TextStream is excused.
                continue;
            if (symbol.startsWith(ns + "bitBlt") || symbol.startsWith(ns + "copyBlt"))
                // you're excused, too
                continue;

            bool symbolOk = false;

            QHash<QString,QStringList>::ConstIterator it = excusedPrefixes.constBegin();
            for ( ; it != excusedPrefixes.constEnd(); ++it) {
                if (!lib.contains(it.key()))
                    continue;
                foreach (QString prefix, it.value())
                    if (symbol.startsWith(prefix)) {
                        symbolOk = true;
                        break;
                    }
            }

            if (symbolOk)
                continue;

            foreach (QString cSymbolPattern, stupidCSymbols)
                if (symbol.contains(cSymbolPattern)) {
                    symbolOk = true;
                    break;
                }

            if (symbolOk)
                continue;

            QStringList fields = symbol.split(' ');
            // the last two fields are address and size and the third last field is the symbol type
            QVERIFY(fields.count() > 3);
            QString type = fields.at(fields.count() - 3);
            // weak symbol
            if (type == QLatin1String("W")) {
                if (symbol.contains("qAtomic"))
                    continue;

                if (symbol.contains("fstat")
                    || symbol.contains("lstat")
                    || symbol.contains("stat64")
                   )
                    continue;

                foreach (QString acceptedPattern, qAlgorithmFunctions + exceptionalSymbols)
                    if (symbol.contains(acceptedPattern)) {
                        symbolOk = true;
                        break;
                    }

                if (symbolOk)
                    continue;

                QString plainSymbol;
                for (int i = 0; i < fields.count() - 3; ++i) {
                    if (i > 0)
                        plainSymbol += QLatin1Char(' ');
                    plainSymbol += fields.at(i);
                }
                foreach (QString qtType, qtTypes)
                    if (plainSymbol.contains(qtType)) {
                        symbolOk = true;
                        break;
                    }

                if (symbolOk)
                    continue;
            }

            QString prefix = ns + "q";
            if (!symbol.startsWith(prefix, Qt::CaseInsensitive)
                && !(isPhonon && symbol.startsWith("Phonon"))) {
                qDebug("symbol in '%s' does not start with prefix '%s': '%s'",
                    qPrintable(lib), qPrintable(prefix), qPrintable(symbol));
                isFailed = true;
            }
        }
    }

#if defined(Q_CC_INTEL)
    QEXPECT_FAIL("", "linux-icc* incorrectly exports some QtWebkit symbols, waiting for a fix from Intel.", Continue);
#endif
    QVERIFY2(!isFailed, "Libraries contain non-prefixed symbols. See Debug output above.");
}
#endif

QTEST_MAIN(tst_Symbols)
#include "tst_symbols.moc"
