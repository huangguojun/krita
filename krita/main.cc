/*
* SPDX-FileCopyrightText: 1999 Matthias Elter <me@kde.org>
* SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
* SPDX-FileCopyrightText: 2015 Boudewijn Rempt <boud@valdyas.org>
*
*  SPDX-License-Identifier: GPL-2.0-or-later
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <stdlib.h>

#include <QString>
#include <QPixmap>
#include <kis_debug.h>
#include <QProcess>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QDir>
#include <QDate>
#include <QLocale>
#include <QSettings>
#include <QByteArray>
#include <QMessageBox>
#include <QThread>
#include <QLibraryInfo>
#include <QTranslator>

#include <QOperatingSystemVersion>

#include <time.h>

#include <KisApplication.h>
#include <KoConfig.h>
#include <KoResourcePaths.h>
#include <kis_config.h>

#include "KisDocument.h"
#include "kis_splash_screen.h"
#include "KisPart.h"
#include "KisApplicationArguments.h"
#include <opengl/kis_opengl.h>
#include "input/KisQtWidgetsTweaker.h"
#include <KisUsageLogger.h>
#include <kis_image_config.h>
#include "KisUiFont.h"

#include <KLocalizedTranslator>


#if defined Q_OS_WIN
#include "config_use_qt_tablet_windows.h"
#include <windows.h>
#ifndef USE_QT_TABLET_WINDOWS
#include <kis_tablet_support_win.h>
#include <kis_tablet_support_win8.h>
#else
#include <dialogs/KisDlgCustomTabletResolution.h>
#endif
#include "config-high-dpi-scale-factor-rounding-policy.h"
#include "config-set-has-border-in-full-screen-default.h"
#ifdef HAVE_SET_HAS_BORDER_IN_FULL_SCREEN_DEFAULT
#include <QtPlatformHeaders/QWindowsWindowFunctions>
#endif
#include <QLibrary>
#endif
#if defined HAVE_KCRASH
#include <kcrash.h>
#elif defined USE_DRMINGW
namespace
{
void tryInitDrMingw()
{
    wchar_t path[MAX_PATH];
    QString pathStr = QCoreApplication::applicationDirPath().replace(L'/', L'\\') + QStringLiteral("\\exchndl.dll");
    if (pathStr.size() > MAX_PATH - 1) {
        return;
    }
    int pathLen = pathStr.toWCharArray(path);
    path[pathLen] = L'\0'; // toWCharArray doesn't add NULL terminator
    HMODULE hMod = LoadLibraryW(path);
    if (!hMod) {
        return;
    }
    // No need to call ExcHndlInit since the crash handler is installed on DllMain
    auto myExcHndlSetLogFileNameA = reinterpret_cast<BOOL (APIENTRY *)(const char *)>(GetProcAddress(hMod, "ExcHndlSetLogFileNameA"));
    if (!myExcHndlSetLogFileNameA) {
        return;
    }
    // Set the log file path to %LocalAppData%\kritacrash.log
    QString logFile = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation).replace(L'/', L'\\') + QStringLiteral("\\kritacrash.log");
    myExcHndlSetLogFileNameA(logFile.toLocal8Bit());
}
} // namespace
#endif

namespace
{

void installTranslators(KisApplication &app);

} // namespace

#ifdef Q_OS_WIN
namespace
{
typedef enum ORIENTATION_PREFERENCE {
    ORIENTATION_PREFERENCE_NONE = 0x0,
    ORIENTATION_PREFERENCE_LANDSCAPE = 0x1,
    ORIENTATION_PREFERENCE_PORTRAIT = 0x2,
    ORIENTATION_PREFERENCE_LANDSCAPE_FLIPPED = 0x4,
    ORIENTATION_PREFERENCE_PORTRAIT_FLIPPED = 0x8
} ORIENTATION_PREFERENCE;
#if !defined(_MSC_VER)
    typedef BOOL WINAPI (*pSetDisplayAutoRotationPreferences_t)(
            ORIENTATION_PREFERENCE orientation
            );
#else
    typedef BOOL (WINAPI *pSetDisplayAutoRotationPreferences_t)(
        ORIENTATION_PREFERENCE orientation
        );
#endif()
void resetRotation()
{
    QLibrary user32Lib("user32");
    if (!user32Lib.load()) {
        qWarning() << "Failed to load user32.dll! This really should not happen.";
        return;
    }
    pSetDisplayAutoRotationPreferences_t pSetDisplayAutoRotationPreferences
            = reinterpret_cast<pSetDisplayAutoRotationPreferences_t>(user32Lib.resolve("SetDisplayAutoRotationPreferences"));
    if (!pSetDisplayAutoRotationPreferences) {
        dbgKrita << "Failed to load function SetDisplayAutoRotationPreferences";
        return;
    }
    bool result = pSetDisplayAutoRotationPreferences(ORIENTATION_PREFERENCE_NONE);
    dbgKrita << "SetDisplayAutoRotationPreferences(ORIENTATION_PREFERENCE_NONE) returned" << result;
}
} // namespace
#endif

#ifdef  Q_OS_WIN
#define MAIN_EXPORT __declspec(dllexport)
#define MAIN_FN krita_main
#else
#define MAIN_EXPORT
#define MAIN_FN main
#endif

extern "C" MAIN_EXPORT int MAIN_FN(int argc, char **argv)
{
    // The global initialization of the random generator
    qsrand(time(0));
    bool runningInKDE = !qgetenv("KDE_FULL_SESSION").isEmpty();

    qputenv("QT_QPA_PLATFORM", "xcb");

    // Workaround a bug in QNetworkManager
    qputenv("QT_BEARER_POLL_TIMEOUT", QByteArray::number(-1));

    // A per-user unique string, without /, because QLocalServer cannot use names with a / in it
    QString key = "Krita5" + QStandardPaths::writableLocation(QStandardPaths::HomeLocation).replace("/", "_");
    key = key.replace(":", "_").replace("\\","_");

    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts, true);

    QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings, true);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);

    QCoreApplication::setAttribute(Qt::AA_DisableShaderDiskCache, true);



    const QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    QSettings kritarc(configPath + QStringLiteral("/kritadisplayrc"), QSettings::IniFormat);

    bool enableOpenGLDebug = false;
    bool openGLDebugSynchronous = false;
    bool logUsage = true;
    {
        if (kritarc.value("EnableHiDPI", true).toBool()) {
            QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        }
        if (!qgetenv("KRITA_HIDPI").isEmpty()) {
            QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        }

        if (!qEnvironmentVariableIsEmpty("KRITA_OPENGL_DEBUG")) {
            enableOpenGLDebug = true;
        } else {
            enableOpenGLDebug = kritarc.value("EnableOpenGLDebug", false).toBool();
        }
        if (enableOpenGLDebug && (qgetenv("KRITA_OPENGL_DEBUG") == "sync" || kritarc.value("OpenGLDebugSynchronous", false).toBool())) {
            openGLDebugSynchronous = true;
        }

        KisConfig::RootSurfaceFormat rootSurfaceFormat = KisConfig::rootSurfaceFormat(&kritarc);
        KisOpenGL::OpenGLRenderer preferredRenderer = KisOpenGL::RendererAuto;

        logUsage = kritarc.value("LogUsage", true).toBool();

        const QString preferredRendererString = kritarc.value("OpenGLRenderer", "auto").toString();
        preferredRenderer = KisOpenGL::convertConfigToOpenGLRenderer(preferredRendererString);

        const KisOpenGL::RendererConfig config =
            KisOpenGL::selectSurfaceConfig(preferredRenderer, rootSurfaceFormat, enableOpenGLDebug);

        KisOpenGL::setDefaultSurfaceConfig(config);
        KisOpenGL::setDebugSynchronous(openGLDebugSynchronous);

    }

    if (logUsage) {
        KisUsageLogger::initialize();
    }


    QString root;
    QString language;
    {
        // Create a temporary application to get the root
        QCoreApplication app(argc, argv);
        Q_UNUSED(app);
        root = KoResourcePaths::getApplicationRoot();
        QSettings languageoverride(configPath + QStringLiteral("/klanguageoverridesrc"), QSettings::IniFormat);
        languageoverride.beginGroup(QStringLiteral("Language"));
        language = languageoverride.value(qAppName(), "").toString();
    }


    {
        QByteArray originalXdgDataDirs = qgetenv("XDG_DATA_DIRS");
        if (originalXdgDataDirs.isEmpty()) {
            // We don't want to completely override the default
            originalXdgDataDirs = "/usr/local/share/:/usr/share/";
        }
        qputenv("XDG_DATA_DIRS", QFile::encodeName(root + "share") + ":" + originalXdgDataDirs);

        // APPIMAGE SOUND ADDITIONS
        // GStreamer needs a few environment variables to properly function in an appimage context.
        // The following code should be configured to **only** run when we detect that Krita is being
        // run within an appimage. Checking for the presence of an APPDIR path env variable seems to be
        // enough to filter out this step for non-appimage krita builds.

        const bool isInAppimage = qEnvironmentVariableIsSet("APPIMAGE");
        if (isInAppimage) {
            QByteArray appimageMountDir = qgetenv("APPDIR");

            //We need to add new gstreamer plugin paths for the system to find the
            //appropriate plugins.
            const QByteArray gstPluginSystemPath = qgetenv("GST_PLUGIN_SYSTEM_PATH_1_0");
            const QByteArray gstPluginScannerPath = qgetenv("GST_PLUGIN_SCANNER");

            //Plugins Path is where libgstreamer-1.0 should expect to find plugin libraries.
            qputenv("GST_PLUGIN_SYSTEM_PATH_1_0", appimageMountDir + QFile::encodeName("/usr/lib/gstreamer-1.0/") + ":" + gstPluginSystemPath);

            //Plugin scanner is where gstreamer should expect to find the plugin scanner.
            //Perhaps invoking the scanenr earlier in the code manually could allow ldd to quickly find all plugin dependencies?
            qputenv("GST_PLUGIN_SCANNER", appimageMountDir + QFile::encodeName("/usr/lib/gstreamer-1.0/gst-plugin-scanner"));
        }
    }

    dbgKrita << "Setting XDG_DATA_DIRS" << qgetenv("XDG_DATA_DIRS");

    // Now that the paths are set, set the language. First check the override from the language
    // selection dialog.

    dbgLocale << "Override language:" << language;
    bool rightToLeft = false;
    if (!language.isEmpty()) {
        KLocalizedString::setLanguages(language.split(":"));

        // And override Qt's locale, too
        QLocale locale(language.split(":").first());
        QLocale::setDefault(locale);
        qputenv("LANG", locale.name().toLocal8Bit());

        const QStringList rtlLanguages = QStringList()
                << "ar" << "dv" << "he" << "ha" << "ku" << "fa" << "ps" << "ur" << "yi";

        if (rtlLanguages.contains(language.split(':').first())) {
            rightToLeft = true;
        }
    }
    else {
        dbgLocale << "Qt UI languages:" << QLocale::system().uiLanguages() << qgetenv("LANG");

        // And if there isn't one, check the one set by the system.
        QLocale locale = QLocale::system();

        if (locale.name() != QStringLiteral("en")) {
            QStringList uiLanguages = locale.uiLanguages();
            for (QString &uiLanguage : uiLanguages) {

                // This list of language codes that can have a specifier should
                // be extended whenever we have translations that need it; right
                // now, only en, pt, zh are in this situation.

                if (uiLanguage.startsWith("en") || uiLanguage.startsWith("pt")) {
                    uiLanguage.replace(QChar('-'), QChar('_'));
                }
                else if (uiLanguage.startsWith("zh-Hant") || uiLanguage.startsWith("zh-TW")) {
                    uiLanguage = "zh_TW";
                }
                else if (uiLanguage.startsWith("zh-Hans") || uiLanguage.startsWith("zh-CN")) {
                    uiLanguage = "zh_CN";
                }
            }

            if (uiLanguages.size() > 0 ) {
                QString envLanguage = uiLanguages.first();
                envLanguage.replace(QChar('-'), QChar('_'));

                for (int i = 0; i < uiLanguages.size(); i++) {
                    QString uiLanguage = uiLanguages[i];
                    // Strip the country code
                    int idx = uiLanguage.indexOf(QChar('-'));

                    if (idx != -1) {
                        uiLanguage = uiLanguage.left(idx);
                        uiLanguages.replace(i, uiLanguage);
                    }
                }
                dbgLocale << "Converted ui languages:" << uiLanguages;
                KLocalizedString::setLanguages(QStringList() << uiLanguages);
                qputenv("LANG", envLanguage.toLocal8Bit());
            }
        }
    }


    // first create the application so we can create a pixmap
    KisApplication app(key, argc, argv);

    installTranslators(app);

    if (app.platformName() == "wayland") {
        QMessageBox::critical(0, i18nc("@title:window", "Fatal Error"), i18n("Krita does not support the Wayland platform. Use XWayland to run Krita on Wayland. Krita will close now."));
        return -1;
    }

    KisUsageLogger::writeHeader();
    KisOpenGL::initialize();

#ifdef HAVE_SET_HAS_BORDER_IN_FULL_SCREEN_DEFAULT
    if (QCoreApplication::testAttribute(Qt::AA_UseDesktopOpenGL)) {
        QWindowsWindowFunctions::setHasBorderInFullScreenDefault(true);
    }
#endif


    if (!language.isEmpty()) {
        if (rightToLeft) {
            app.setLayoutDirection(Qt::RightToLeft);
        }
        else {
            app.setLayoutDirection(Qt::LeftToRight);
        }
    }

    KLocalizedString::setApplicationDomain("krita");

    dbgLocale << "Available translations" << KLocalizedString::availableApplicationTranslations();
    dbgLocale << "Available domain translations" << KLocalizedString::availableDomainTranslations("krita");



    if (qApp->applicationDirPath().contains(KRITA_BUILD_DIR)) {
        qFatal("FATAL: You're trying to run krita from the build location. You can only run Krita from the installation location.");
    }


#if defined HAVE_KCRASH
    KCrash::initialize();
#elif defined USE_DRMINGW
    tryInitDrMingw();
#endif

    KisApplicationArguments args(app);

    if (app.isRunning()) {
        // only pass arguments to main instance if they are not for batch processing
        // any batch processing would be done in this separate instance
        const bool batchRun = args.exportAs() || args.exportSequence();

        if (!batchRun) {
            QByteArray ba = args.serialize();
            if (app.sendMessage(ba)) {
                return 0;
            }
        }
    }

    if (!runningInKDE) {
        // Icons in menus are ugly and distracting
        app.setAttribute(Qt::AA_DontShowIconsInMenus);
    }
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    app.setAttribute(Qt::AA_DisableWindowContextHelpButton);
#endif
    app.installEventFilter(KisQtWidgetsTweaker::instance());

    if (!args.noSplash()) {
        QWidget *splash = new KisSplashScreen();
        app.setSplashScreen(splash);
    }

    app.setAttribute(Qt::AA_CompressHighFrequencyEvents, false);

    // Set up remote arguments.
    QObject::connect(&app, SIGNAL(messageReceived(QByteArray,QObject*)),
                     &app, SLOT(remoteArguments(QByteArray,QObject*)));

    QObject::connect(&app, SIGNAL(fileOpenRequest(QString)),
                     &app, SLOT(fileOpenRequested(QString)));

    // Hardware information
    KisUsageLogger::writeSysInfo("\nHardware Information\n");
    KisUsageLogger::writeSysInfo(QString("  GPU Acceleration: %1").arg(kritarc.value("OpenGLRenderer", "auto").toString()));
    KisUsageLogger::writeSysInfo(QString("  Memory: %1 Mb").arg(KisImageConfig(true).totalRAM()));
    KisUsageLogger::writeSysInfo(QString("  Number of Cores: %1").arg(QThread::idealThreadCount()));
    KisUsageLogger::writeSysInfo(QString("  Swap Location: %1\n").arg(KisImageConfig(true).swapDir()));

    KisConfig(true).logImportantSettings();

    app.setFont(KisUiFont::normalFont());

    if (!app.start(args)) {
        KisUsageLogger::log("Could not start Krita Application");
        return 1;
    }


    int state = app.exec();

    {
        QSettings kritarc(configPath + QStringLiteral("/kritadisplayrc"), QSettings::IniFormat);
        kritarc.setValue("canvasState", "OPENGL_SUCCESS");
    }

    if (logUsage) {
        KisUsageLogger::close();
    }

    return state;
}

namespace
{

void removeInstalledTranslators(KisApplication &app)
{
    // HACK: We try to remove all the translators installed by ECMQmLoader.
    // The reason is that it always load translations for the system locale
    // which interferes with our effort to handle override languages. Since
    // `en_US` (or `en`) strings are defined in code, the QTranslator doesn't
    // actually handle translations for them, so even if we try to install
    // a QTranslator loaded from `en`, the strings always get translated by
    // the system language QTranslator that ECMQmLoader installed instead
    // of the English one.

    // ECMQmLoader creates all QTranslator's parented to the active QApp.
    QList<QTranslator *> translators = app.findChildren<QTranslator *>(QString(), Qt::FindDirectChildrenOnly);
    Q_FOREACH(const auto &translator, translators) {
        app.removeTranslator(translator);
    }
    dbgLocale << "Removed" << translators.size() << "QTranslator's";
}

void installPythonPluginUITranslator(KisApplication &app)
{
    // Install a KLocalizedTranslator, so that when the bundled Python plugins
    // load their UI files using uic.loadUi() it can be translated.
    // These UI files must specify "pykrita_plugin_ui" as their class names.
    KLocalizedTranslator *translator = new KLocalizedTranslator(&app);
    translator->setObjectName(QStringLiteral("KLocalizedTranslator.pykrita_plugin_ui"));
    translator->setTranslationDomain(QStringLiteral("krita"));
    translator->addContextToMonitor(QStringLiteral("pykrita_plugin_ui"));
    app.installTranslator(translator);
}

void installQtTranslations(KisApplication &app)
{
    QStringList qtCatalogs = {
        QStringLiteral("qt_"),
        QStringLiteral("qtbase_"),
        QStringLiteral("qtmultimedia_"),
        QStringLiteral("qtdeclarative_"),
    };
    // A list of locale to add, note that the last added one has the
    // highest precedence.
    QList<QLocale> localeList;
    // We always use English as the final fallback.
    localeList.append(QLocale(QLocale::English));
    QLocale defaultLocale;
    if (defaultLocale.language() != QLocale::English) {
        localeList.append(defaultLocale);
    }

    QString translationsPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    dbgLocale << "Qt translations path:" << translationsPath;

    Q_FOREACH(const auto &localeToLoad, localeList) {
        Q_FOREACH(const auto &catalog, qtCatalogs) {
            QTranslator *translator = new QTranslator(&app);
            if (translator->load(localeToLoad, catalog, QString(), translationsPath)) {
                dbgLocale << "Loaded Qt translations for" << localeToLoad << catalog;
                translator->setObjectName(QStringLiteral("QTranslator.%1.%2").arg(localeToLoad.name(), catalog));
                app.installTranslator(translator);
            } else {
                delete translator;
            }
        }
    }
}

void installEcmTranslations(KisApplication &app)
{
    // Load translations created using the ECMPoQmTools module.
    // This function is based on the code in:
    // https://invent.kde.org/frameworks/extra-cmake-modules/-/blob/master/modules/ECMQmLoader.cpp.in

    QStringList ecmCatalogs = {
        QStringLiteral("kcompletion5_qt"),
        QStringLiteral("kconfig5_qt"),
        QStringLiteral("kcoreaddons5_qt"),
        QStringLiteral("kitemviews5_qt"),
        QStringLiteral("kwidgetsaddons5_qt"),
        QStringLiteral("kwindowsystem5_qt"),
        QStringLiteral("seexpr2_qt"),
    };

    QStringList ki18nLangs = KLocalizedString::languages();
    const QString langEn = QStringLiteral("en");
    // Replace "en_US" with "en" because that's what we have in the locale dir.
    int indexOfEnUs = ki18nLangs.indexOf(QStringLiteral("en_US"));
    if (indexOfEnUs != -1) {
        ki18nLangs[indexOfEnUs] = langEn;
    }
    // We need to have "en" to the end of the list, because we explicitly
    // removed the "en" translators added by ECMQmLoader.
    // If "en" is already on the list, we truncate the ones after, because
    // "en" is the catch-all fallback that has the strings in code.
    int indexOfEn = ki18nLangs.indexOf(langEn);
    if (indexOfEn != -1) {
        for (int i = ki18nLangs.size() - indexOfEn - 1; i > 0; i--) {
            ki18nLangs.removeLast();
        }
    } else {
        ki18nLangs.append(langEn);
    }

    // The last added one has the highest precedence, so we iterate the
    // list backwards.
    QStringListIterator langIter(ki18nLangs);
    langIter.toBack();

    while (langIter.hasPrevious()) {
        const QString &localeDirName = langIter.previous();
        Q_FOREACH(const auto &catalog, ecmCatalogs) {
            QString subPath = QStringLiteral("locale/") % localeDirName % QStringLiteral("/LC_MESSAGES/") % catalog % QStringLiteral(".qm");
            // Our patched k18n uses AppDataLocation (for AppImage).
            QString fullPath = QStandardPaths::locate(QStandardPaths::AppDataLocation, subPath);
            if (fullPath.isEmpty()) {
                // ... but distro builds probably still use GenericDataLocation,
                // so check that too.
                fullPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, subPath);
                if (fullPath.isEmpty()) {
                    continue;
                }
            }
            QTranslator *translator = new QTranslator(&app);
            if (translator->load(fullPath)) {
                dbgLocale << "Loaded ECM translations for" << localeDirName << catalog;
                translator->setObjectName(QStringLiteral("QTranslator.%1.%2").arg(localeDirName, catalog));
                app.installTranslator(translator);
            } else {
                delete translator;
            }
        }
    }
}

void installTranslators(KisApplication &app)
{
    removeInstalledTranslators(app);
    installPythonPluginUITranslator(app);
    installQtTranslations(app);
    installEcmTranslations(app);
}

} // namespace
