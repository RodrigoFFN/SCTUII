#include "SimulationController.hpp"
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QTemporaryDir>
#include <QFile>
#include <QThread>
#include <functional>
#include <iostream>

namespace {
bool waitUntil(const std::function<bool()>& predicate, int timeout=10000) {
    QElapsedTimer timer; timer.start();
    while (!predicate() && timer.elapsed()<timeout) {
        QCoreApplication::processEvents(); QThread::msleep(5);
    }
    QCoreApplication::processEvents(); return predicate();
}
#define CHECK(expr) do { if (!(expr)) { std::cerr << "Error: " #expr << '\n'; return 1; } } while(false)
}
int main(int argc,char** argv) {
    QCoreApplication app(argc,argv);
    SimulationController c;
    CHECK(!c.status().value("running").toBool());
    CHECK(!c.start(0,20,60,200,20,false));
    CHECK(!c.message().isEmpty());
    for (bool adaptive : {false,true}) {
        CHECK(c.start(4,30,100,300,50,adaptive));
        CHECK(waitUntil([&]{return c.vehicles()->rowCount()>0;}));
        CHECK(waitUntil([&]{return c.status().value("finished").toBool();}));
        CHECK(c.status().value("served").toInt()==8);
        CHECK(c.status().value("pending").toInt()==0);
        CHECK(c.history().size()==8);
        CHECK(c.vehicles()->rowCount()==8);
        CHECK(c.eventCount()>0);
        QTemporaryDir dir; CHECK(dir.isValid());
        const auto path=dir.filePath("eventos.csv");
        CHECK(c.exportCsv(QUrl::fromLocalFile(path)));
        QFile f(path); CHECK(f.open(QIODevice::ReadOnly));
        const auto csv=f.readAll(); CHECK(csv.contains("tipo,eje,id,tiempo_ms,descripcion")); CHECK(csv.contains("\"paso\""));
        c.reset(); CHECK(c.eventCount()==0); CHECK(c.vehicles()->rowCount()==0);
    }
    CHECK(c.start(60,10,20,100,20,true));
    CHECK(!c.start(3,30,60,200,50,false));
    CHECK(waitUntil([&]{return c.status().value("finished").toBool();}));
    CHECK(c.status().value("served").toInt()==120);
    CHECK(c.history().size()==120); CHECK(c.vehicles()->rowCount()==120);
    CHECK(!c.exportCsv(QUrl::fromLocalFile("/nonexistent-simtraffic-path/eventos.csv")));
    CHECK(!c.message().isEmpty());
    c.reset();
    CHECK(c.start(60,20,200,500,50,true));
    CHECK(waitUntil([&]{return c.status().value("generated").toInt()>2;}));
    c.stop(); CHECK(waitUntil([&]{return !c.status().value("running").toBool();}));
    CHECK(c.status().value("stopped").toBool());
    c.reset(); CHECK(c.status().value("generated").toInt()==0);
    // Cierre con motor activo: el destructor debe cancelar y unir los hilos.
    { SimulationController active; CHECK(active.start(60,100,200,500,50,false)); }
    return 0;
}
