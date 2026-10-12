#include <QFontDatabase>
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QTimer>

#include "watermark.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName("img-settings");
    app.setApplicationDisplayName("img-settings");
    app.setOrganizationDomain("img-settings.io");
    app.setApplicationVersion(APP_VERSION);
    app.setWindowIcon(QIcon(":/assets/AppIcon.png"));

    // The watermark keeps using Go Regular so output matches earlier releases.
    const int fontId = QFontDatabase::addApplicationFont(":/assets/Go-Regular.ttf");
    Watermark::setFontFamily(QFontDatabase::applicationFontFamilies(fontId).value(0));

    // Windows gets the native Windows 11 (WinUI 3) controls; elsewhere the Basic
    // style is fully customizable, so the controls can be drawn as Liquid Glass.
    // IMG_SETTINGS_STYLE=windows previews the Windows look on other platforms.
#ifdef Q_OS_WIN
    const bool fluent = true;
#else
    const bool fluent = qEnvironmentVariable("IMG_SETTINGS_STYLE") == QLatin1String("windows");
#endif
    QQuickStyle::setStyle(fluent ? "FluentWinUI3" : "Basic");

    QQmlApplicationEngine engine;
    engine.setInitialProperties({{"fluent", fluent}});
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app,
                     [] { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
    engine.loadFromModule("ImgSettings", "Main");

    // --smoke-test: used by CI to check that a deployed build can load its UI.
    if (app.arguments().contains(QLatin1String("--smoke-test"))) {
        if (engine.rootObjects().isEmpty())
            return 1;
        QTimer::singleShot(1500, &app, &QCoreApplication::quit);
    }

    return app.exec();
}
