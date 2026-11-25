#include "SimulationController.hpp"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QTimer>
#include <QWindow>

int main(int argc,char** argv) {
    QGuiApplication app(argc,argv);
    app.setApplicationName("SimTrrafic"); app.setOrganizationName("SCTUII");
    QQuickStyle::setStyle("Basic");
    SimulationController controller;
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("simulation", &controller);
    engine.load(QUrl(QStringLiteral("qrc:/simtrrafic/qml/Main.qml")));
    if (engine.rootObjects().isEmpty()) return 1;
    if (app.arguments().contains("--smoke-test")) {
        QTimer::singleShot(100,&app,[&] {
            if (!controller.start(3,90,120,400,60,false)) app.exit(2);
        });
        QTimer::singleShot(450,&app,[&] {
            if (auto* window=qobject_cast<QWindow*>(engine.rootObjects().first())) window->resize(1280,720);
            controller.stop();
        });
        QTimer::singleShot(1200,&app,[&] {
            if (controller.status().value("running").toBool()) { app.exit(3); return; }
            controller.reset(); app.exit(controller.eventCount()==0?0:4);
        });
    }
    return app.exec();
}
