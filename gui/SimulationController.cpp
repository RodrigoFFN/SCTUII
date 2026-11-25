#include "SimulationController.hpp"
#include <QSaveFile>
#include <QTextStream>
#include <QStringConverter>
#include <algorithm>

namespace {
QString axisName(Eje axis) { return axis == Eje::NorteSur ? QStringLiteral("N–S") : QStringLiteral("E–O"); }
QString csv(QString value) { value.replace('"', QStringLiteral("\"\"")); return QStringLiteral("\"") + value + QStringLiteral("\""); }
}
int EventLogModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : static_cast<int>(events_.size());
}
QVariant EventLogModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row()<0 || index.row()>=rowCount()) return {};
    const auto& event=events_[events_.size()-1-static_cast<size_t>(index.row())];
    switch (role) {
    case Time: return event.transcurridosMs;
    case Type: return QString::fromStdString(event.tipo);
    case Axis: return event.vehiculoId || (event.tipo=="fase" && event.descripcion.find("VERDE")!=std::string::npos)
                    ? axisName(event.eje) : QStringLiteral("—");
    case Id: return event.vehiculoId;
    case Description: return QString::fromStdString(event.descripcion);
    default: return {};
    }
}
QHash<int,QByteArray> EventLogModel::roleNames() const {
    return {{Time,"eventTime"},{Type,"eventType"},{Axis,"eventAxis"},{Id,"eventId"},{Description,"eventDescription"}};
}
void EventLogModel::append(const std::vector<EventoSimulacion>& events) {
    if (events.size()<=events_.size()) return;
    const size_t first=events_.size();
    beginInsertRows({},0,static_cast<int>(events.size()-first)-1);
    events_.insert(events_.end(),events.begin()+first,events.end());
    endInsertRows();
}
void EventLogModel::clear() {
    if (events_.empty()) return;
    beginResetModel(); events_.clear(); endResetModel();
}
SimulationController::SimulationController(QObject* parent) : QObject(parent) {
    clock_.start(); snapshotClock_.start();
    connect(&pollTimer_, &QTimer::timeout, this, &SimulationController::poll);
    connect(&frameTimer_, &QTimer::timeout, this, &SimulationController::animate);
    pollTimer_.setTimerType(Qt::PreciseTimer);
    frameTimer_.setTimerType(Qt::PreciseTimer);
    pollTimer_.setInterval(35); frameTimer_.setInterval(16); poll();
}
SimulationController::~SimulationController() {
    pollTimer_.stop(); frameTimer_.stop();
    if (simulator_ && monitor_.ejecutando()) {
        cancel_.store(true); simulator_->solicitarDetencion();
    }
    join();
}
void SimulationController::join() { if (worker_.joinable()) worker_.join(); }
void SimulationController::setMessage(const QString& text, bool error) {
    if (message_ == text && messageIsError_ == error) return;
    message_ = text; messageIsError_ = error; emit messageChanged();
}
void SimulationController::clearMessage() { setMessage({}); }
bool SimulationController::start(int count, int arrival, int crossing, int green, int red, bool adaptive) {
    if (monitor_.ejecutando()) return false;
    join(); clearMessage();
    try {
        ConfiguracionSimulacion c;
        c.vehiculosPorEje=count; c.intervaloLlegadaMs=arrival; c.tiempoCruceMs=crossing;
        c.duracionVerdeMs=green; c.duracionTodoRojoMs=red; c.adaptativo=adaptive; c.mostrarConsola=false;
        c.validar();
        simulator_ = std::make_shared<Simulador>(c, [this](const auto& e, const auto& stats, const auto& state) {
            monitor_.registrar(e,stats,state);
        });
        cancel_.store(false); stopping_=false;
        monitor_.iniciar(c);
        auto simulator=simulator_;
        worker_=std::thread([this,simulator] {
            try { auto result=simulator->ejecutar(); monitor_.finalizar(result,cancel_.load()); }
            catch (const std::exception& e) { monitor_.fallar(e.what()); }
            catch (...) { monitor_.fallar("Error desconocido del simulador"); }
        });
        poll();
        if (snapshot_.ejecutando) pollTimer_.start();
        if (snapshot_.ejecutando || snapshot_.terminado) { previousFrame_=clock_.elapsed(); frameTimer_.start(); }
        return true;
    } catch (const std::exception& e) {
        if (monitor_.ejecutando()) monitor_.fallar(e.what());
        setMessage(QString::fromUtf8(e.what()),true); poll(); return false;
    }
}
void SimulationController::stop() {
    if (!simulator_ || !monitor_.ejecutando() || stopping_) return;
    stopping_=true; cancel_.store(true); simulator_->solicitarDetencion(); emit statusChanged();
}
void SimulationController::reset() {
    if (monitor_.ejecutando()) return;
    join(); simulator_.reset(); stopping_=false; clearMessage(); monitor_.reiniciarVista(); poll();
}
void SimulationController::setSceneExtent(double horizontal,double vertical) {
    vehicles_.setExtent(horizontal,vertical);
    if (!frameTimer_.isActive()) animate();
}
void SimulationController::setReducedMotion(bool reduced) {
    vehicles_.setReducedMotion(reduced);
    if (!frameTimer_.isActive()) animate();
}
void SimulationController::poll() {
    auto s=monitor_.instantanea();
    if (run_ != s.ejecucion) {
        run_=s.ejecucion; eventCount_=0; eventModel_.clear(); history_.clear();
        emit eventsChanged(); emit historyChanged();
    }
    if (!s.ejecutando) {
        stopping_=false; pollTimer_.stop();
        if (!s.terminado) frameTimer_.stop();
    }
    const auto& r=s.resumen;
    const QString phase=s.estado.norteSur==Luz::Verde ? "ns" : s.estado.esteOeste==Luz::Verde ? "eo" : "rojo";
    QVariantMap next{{"runId",QVariant::fromValue<qulonglong>(s.ejecucion)},{"running",s.ejecutando},{"finished",s.terminado},{"stopped",s.detenido},
        {"phase",phase},{"occupied",s.estado.ocupado},{"elapsed",QVariant::fromValue<qlonglong>(s.tiempoMs)},
        {"generated",r.totalGenerados()},{"served",r.totalAtendidos()},{"pending",r.pendientes()},
        {"wait",r.esperaPromedio()},{"nsGenerated",r.generadosNS},{"nsServed",r.atendidosNS},
        {"eoGenerated",r.generadosEO},{"eoServed",r.atendidosEO},{"nsWait",r.esperaPromedioNS()},
        {"eoWait",r.esperaPromedioEO()},{"expected",2*s.config.vehiculosPorEje},
        {"adaptive",s.config.adaptativo},{"error",QString::fromStdString(s.error)}};
    if (next!=status_) { status_=next; emit statusChanged(); }
    if (!s.error.empty()) setMessage(QString::fromStdString(s.error),true);
    if (s.historial.size()!=eventCount_) {
        eventModel_.append(s.historial);
        for (size_t i=eventCount_;i<s.historial.size();++i) {
            const auto& e=s.historial[i];
            if (e.tipo=="paso") {
                history_.append(QVariantMap{{"time",e.transcurridosMs},{"total",static_cast<int>(history_.size())+1}});
            }
        }
        eventCount_=s.historial.size(); emit eventsChanged(); emit historyChanged();
    }
    // El reloj visual interpola entre instantaneas; conserva el tiempo al terminar.
    const bool clockChanged=s.ejecutando || snapshot_.ejecutando || s.ejecucion!=snapshot_.ejecucion;
    snapshot_=std::move(s); if (clockChanged) snapshotClock_.restart();
    const int ns=hiddenNS(), eo=hiddenEO();
    vehicles_.sync(snapshot_);
    if (ns!=hiddenNS() || eo!=hiddenEO()) emit queuesChanged();
}
void SimulationController::animate() {
    const qint64 now=clock_.elapsed();
    const double dt=std::clamp((now-previousFrame_)/1000.0,0.0,0.05); previousFrame_=now;
    const int ns=hiddenNS(), eo=hiddenEO();
    const double elapsed=snapshot_.tiempoMs+std::min<qint64>(snapshotClock_.elapsed(), snapshot_.ejecutando ? 50 : 3000);
    vehicles_.frame(dt,elapsed);
    if (ns!=hiddenNS() || eo!=hiddenEO()) emit queuesChanged();
    if (!snapshot_.ejecutando && (!snapshot_.terminado || snapshotClock_.elapsed()>3100)) frameTimer_.stop();
}
bool SimulationController::exportCsv(const QUrl& file) {
    if (!file.isLocalFile() || eventCount()==0) return false;
    QSaveFile output(file.toLocalFile());
    if (!output.open(QIODevice::WriteOnly|QIODevice::Text)) { setMessage(output.errorString(),true); return false; }
    QTextStream out(&output); out.setEncoding(QStringConverter::Utf8);
    out << "tipo,eje,id,tiempo_ms,descripcion\n";
    const auto s=monitor_.instantanea();
    for (const auto& e:s.historial) {
        out << csv(QString::fromStdString(e.tipo)) << ',' << csv(e.vehiculoId || (e.tipo=="fase" && e.descripcion.find("VERDE")!=std::string::npos) ? axisName(e.eje) : QStringLiteral("—")) << ',' << e.vehiculoId << ','
            << e.transcurridosMs << ',' << csv(QString::fromStdString(e.descripcion)) << '\n';
    }
    out.flush();
    if (out.status()!=QTextStream::Ok || !output.commit()) { setMessage("No se pudo guardar el CSV.",true); return false; }
    setMessage("Bitácora exportada correctamente."); return true;
}
