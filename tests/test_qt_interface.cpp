#include "SimulationController.hpp"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlError>
#include <QQuickWindow>
#include <QQuickItem>
#include <QQuickStyle>
#include <QTest>
#include <QElapsedTimer>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QPalette>
#include <functional>
#include <iostream>

#define CHECK(e) do { if (!(e)) { std::cerr << "Error: " #e << " (line " << __LINE__ << ")\n"; return 1; } } while(false)
static bool until(const std::function<bool()>& pred, int timeout=30000) {
    QElapsedTimer timer; timer.start();
    while (!pred() && timer.elapsed()<timeout) QTest::qWait(10);
    return pred();
}
int main(int argc,char** argv) {
    QGuiApplication app(argc,argv);
    QQuickStyle::setStyle("Basic");
    // Reproduce un escritorio oscuro: la interfaz tiene que conservar su paleta.
    QPalette dark; dark.setColor(QPalette::Window,Qt::black); dark.setColor(QPalette::Button,Qt::black);
    dark.setColor(QPalette::Base,Qt::black); dark.setColor(QPalette::Text,Qt::white); app.setPalette(dark);
    SimulationController simulation;
    QQmlApplicationEngine engine;
    QStringList warnings;
    QObject::connect(&engine,&QQmlApplicationEngine::warnings,[&](const QList<QQmlError>& errors) {
        for(const auto& error:errors) warnings<<error.toString();
    });
    engine.rootContext()->setContextProperty("simulation",&simulation);
    engine.load(QUrl::fromLocalFile(QStringLiteral(SIMTRAFFIC_QML_DIR "/Main.qml")));
    CHECK(!engine.rootObjects().isEmpty());
    auto* window=qobject_cast<QQuickWindow*>(engine.rootObjects().first()); CHECK(window);
    auto find=[&](const char* name){return window->findChild<QQuickItem*>(QString::fromUtf8(name));};
    auto click=[&](const char* name){
        auto* item=find(name); if (!item || !item->isEnabled()) return false;
        const auto pt=item->mapToScene(QPointF(item->width()/2,item->height()/2)).toPoint();
        QTest::mouseClick(window,Qt::LeftButton,Qt::NoModifier,pt); QTest::qWait(30); return true;
    };
    const auto baseDir=qEnvironmentVariable("SIMTRAFFIC_CAPTURE_DIR");
    const auto captureDir=baseDir.isEmpty() ? QString{} : baseDir+(window->devicePixelRatio()>1 ? "/hidpi" : "");
    auto capture=[&](const QString& name){
        if(captureDir.isEmpty()) return true;
        QDir().mkpath(captureDir); QTest::qWait(100);
        auto image=window->grabWindow();
        return !image.isNull() && image.save(captureDir+"/"+name+".png");
    };
    QTest::qWait(250);
    CHECK(find("startButton")); CHECK(find("greenField")); CHECK(find("policyBox"));
    CHECK(!simulation.status()["running"].toBool()); CHECK(simulation.eventCount()==0);
    for (auto size:{QSize(1280,720),QSize(1366,768),QSize(1920,1080)}) {
        window->resize(size); QTest::qWait(100);
        CHECK(find("mapViewport")->width()>600);
        CHECK(find("mapViewport")->height()>260);
        CHECK(find("greenField")->width()>=120);
        CHECK(capture(QString("inicio_%1x%2").arg(size.width()).arg(size.height())));
    }
    window->resize(1366,768); QTest::qWait(80);
    // Entrada editable, cinco dígitos, confirmación y navegación de teclado.
    CHECK(click("greenField")); QTest::keyClick(window,Qt::Key_A,Qt::ControlModifier);
    for(char digit:std::string("10000")) QTest::keyClick(window,static_cast<Qt::Key>(digit)); QTest::keyClick(window,Qt::Key_Tab); QTest::qWait(30);
    CHECK(find("greenField")->property("value").toInt()==10000);
    CHECK(window->activeFocusItem()!=nullptr);
    CHECK(click("collapseLog")); CHECK(find("eventLog")->property("collapsed").toBool());
    CHECK(click("collapseLog")); CHECK(!find("eventLog")->property("collapsed").toBool());
    CHECK(click("reduceMotion")); CHECK(find("reduceMotion")->property("checked").toBool());
    CHECK(click("reduceMotion"));
    if(app.arguments().contains("--layout-only")) {
        CHECK(warnings.isEmpty()); return 0;
    }
    // Seleccionar cada escenario a través del ComboBox y arrancar con su botón real.
    for(int scenario=0;scenario<3;++scenario) {
        auto* combo=find("scenarioBox"); CHECK(combo);
        CHECK(click("scenarioBox"));
        auto* popup=combo->property("popup").value<QObject*>(); CHECK(popup);
        CHECK(popup->property("visible").toBool());
        auto* list=popup->property("contentItem").value<QQuickItem*>(); CHECK(list);
        QQuickItem* option=nullptr;
        CHECK(QMetaObject::invokeMethod(list,"itemAtIndex",Q_RETURN_ARG(QQuickItem*,option),Q_ARG(int,scenario)));
        CHECK(option);
        QTest::mouseClick(option->window(),Qt::LeftButton,Qt::NoModifier,option->mapToScene(QPointF(option->width()/2,option->height()/2)).toPoint());
        QTest::qWait(80);
        CHECK(combo->property("currentIndex").toInt()==scenario);
        CHECK(find("countField")->property("value").toInt()==(scenario==0?3:scenario==1?6:16));
        CHECK(click("startButton")); CHECK(simulation.status()["running"].toBool());
        CHECK(!find("countField")->isEnabled()); CHECK(!find("startButton")->isEnabled());
        bool ns=false,eo=false,occupied=false,queue=false;
        QElapsedTimer timer;timer.start();
        bool captured=false;
        while(simulation.status()["running"].toBool() && timer.elapsed()<40000) {
            QTest::qWait(35); const auto s=simulation.status();
            ns|=s["phase"].toString()=="ns"; eo|=s["phase"].toString()=="eo";
            occupied|=s["occupied"].toBool(); queue|=s["pending"].toInt()>1;
            if(!captured && s["served"].toInt()>=2 && s["occupied"].toBool()) {
                CHECK(capture(QString("escenario_%1").arg(scenario))); captured=true;
            }
        }
        CHECK(!simulation.status()["running"].toBool()); CHECK(simulation.status()["finished"].toBool());
        CHECK(ns && eo && occupied && queue);
        CHECK(simulation.status()["served"].toInt()==(scenario==0?6:scenario==1?12:32));
        CHECK(simulation.status()["pending"].toInt()==0);
        CHECK(simulation.history().size()==simulation.status()["served"].toInt());
        QTemporaryDir dir; const auto path=dir.filePath("eventos.csv");
        auto* dialog=window->findChild<QObject*>("exportDialog"); CHECK(dialog);
        CHECK(dialog->setProperty("currentFolder",QUrl::fromLocalFile(dir.path())));
        CHECK(dialog->setProperty("selectedFile",QUrl::fromLocalFile(path)));
        CHECK(click("exportButton"));
        CHECK(dialog->property("visible").toBool());
        CHECK(QMetaObject::invokeMethod(dialog,"accept"));
        CHECK(until([&]{return QFile::exists(path);},2000));
        QFile csv(path);CHECK(csv.open(QIODevice::ReadOnly));
        CHECK(csv.readAll().count('\n')==simulation.eventCount()+1);
        CHECK(click("resetButton"));CHECK(simulation.eventCount()==0);CHECK(simulation.vehicles()->rowCount()==0);
    }
    CHECK(click("startButton"));
    CHECK(until([&]{return simulation.status()["generated"].toInt()>3;}));
    CHECK(click("stopButton"));CHECK(until([&]{return !simulation.status()["running"].toBool();}));
    CHECK(simulation.status()["stopped"].toBool());CHECK(click("resetButton"));
    CHECK(click("startButton"));CHECK(simulation.status()["running"].toBool());
    window->close(); // El destructor del controlador cancela y une los hilos al salir.
    for(const auto& warning:warnings) std::cerr << warning.toStdString() << '\n';
    CHECK(warnings.isEmpty());
    std::cout << "QML: escenarios bajo/normal/intenso, fases, colas, CSV, teclado, tamaños, parada, reinicio y cierre OK\n";
    return 0;
}
