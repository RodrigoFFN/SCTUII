#include "VehicleModel.hpp"
#include <algorithm>
#include <cmath>
#include <iterator>

VehicleModel::VehicleModel(QObject* parent) : QAbstractListModel(parent) {}
int VehicleModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : static_cast<int>(rows_.size());
}
QVariant VehicleModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= rowCount()) return {};
    const auto& r = rows_[index.row()];
    switch (role) {
    case Id: return r.id;
    case Axis: return r.ns ? QStringLiteral("ns") : QStringLiteral("eo");
    case Bus: return r.bus;
    case Position: return r.position;
    case Visible: return r.visible;
    case Stage: return r.stage;
    default: return {};
    }
}
QHash<int, QByteArray> VehicleModel::roleNames() const {
    return {{Id,"vehicleId"},{Axis,"axis"},{Bus,"isBus"},{Position,"travel"},
            {Visible,"onMap"},{Stage,"vehicleStage"}};
}
void VehicleModel::clear() {
    beginResetModel(); rows_.clear(); hiddenNS_ = hiddenEO_ = 0; endResetModel();
}
void VehicleModel::setExtent(double horizontal, double vertical) {
    horizontal_ = std::max(150.0, horizontal);
    vertical_ = std::max(150.0, vertical);
}
void VehicleModel::sync(const MonitorSimulacion::Instantanea& s) {
    if (run_ != s.ejecucion) { clear(); run_ = s.ejecucion; }
    crossingMs_ = s.config.tiempoCruceMs;
    stopped_ = s.terminado && (s.detenido || !s.error.empty());
    for (const auto& [id, v] : s.vehiculos) {
        auto it = std::find_if(rows_.begin(), rows_.end(), [id](const Row& r){ return r.id == id; });
        if (it == rows_.end()) {
            Row r;
            r.id = id; r.ns = v.eje == Eje::NorteSur; r.bus = v.tipo == "Bus";
            r.position = -(r.ns ? vertical_ : horizontal_) - r.halfLength();
            const int n = rowCount();
            beginInsertRows({}, n, n); rows_.push_back(r); endInsertRows();
            it = std::prev(rows_.end());
        }
        const bool changed = it->stage != v.etapa;
        if (changed && v.etapa == 2) it->crossStart = std::min(-94-it->halfLength(),it->position);
        it->stage = v.etapa; it->arrival = v.llegada; it->cross = v.cruce; it->served = v.salida;
        if (changed) { const auto i = index(static_cast<int>(it-rows_.begin()),0); emit dataChanged(i,i,{Stage}); }
    }
    // El identificador crece dentro de cada eje: conserva el orden de llegada.
    for (bool ns : {true, false}) {
        std::vector<Row*> queue;
        for (auto& r : rows_) if (r.ns == ns && r.stage == 1) queue.push_back(&r);
        std::sort(queue.begin(), queue.end(), [](auto* a, auto* b){ return a->id < b->id; });
        double tail = -94;
        for (auto* r : queue) {
            r->queue = tail - r->halfLength();
            tail -= 2 * r->halfLength() + 8;
        }
    }
}
void VehicleModel::frame(double dt, double elapsed) {
    hiddenNS_ = hiddenEO_ = 0;
    for (bool ns : {true, false}) {
        std::vector<Row*> lane;
        for (auto& r : rows_) if (r.ns == ns) lane.push_back(&r);
        std::sort(lane.begin(), lane.end(), [](auto* a, auto* b){ return a->id < b->id; });
        double rear = 1e6;
        const double edge = ns ? vertical_ : horizontal_;
        for (auto* r : lane) {
            const double half = r->halfLength(), entry = -94-half, exit = 94+half;
            const double old = r->position;
            const bool wasVisible = r->visible;
            if (r->stage == 1) {
                // Suaviza la aproximacion, sin atravesar la linea de detencion.
                const double target = std::min(r->queue, rear-half-8);
                const double visualDt = r->fresh && r->arrival >= 0
                    ? std::max(dt,std::clamp((elapsed-r->arrival)/1000.0,0.0,0.05)) : dt;
                const double alpha = reduced_ ? 1.0 : 1-std::exp(-visualDt/0.12);
                r->position += (target-r->position)*(stopped_ ? 1.0 : alpha);
                r->position = std::min(r->position, rear-half-8);
            } else if (r->stage == 2 && r->cross >= 0) {
                const double t = std::clamp((elapsed-r->cross)/std::max(1,crossingMs_), 0.0, 1.0);
                // No completa el cruce antes de recibir el evento real de paso.
                const double eased = t*t*(3-2*t);
                r->position = reduced_ ? 0.0 : r->crossStart+(exit-r->crossStart)*std::min(eased,0.97);
            } else if (r->stage == 3 && r->served >= 0) {
                const double speed = std::max(450.0, (188.0+60.0)*1000.0/std::max(1,crossingMs_));
                r->position = std::max(exit,std::min(exit+std::max(0.0,elapsed-r->served)*speed/1000.0,rear-half-8));
            }
            r->fresh = false;
            // Un vehiculo ya atendido nunca ocupa visualmente la zona critica.
            r->visible = r->position+half > -edge && r->position-half < edge;
            if (r->stage == 1 && r->queue+half <= -edge) {
                r->visible = false;
                if (ns) ++hiddenNS_; else ++hiddenEO_;
            }
            if (r->stage == 1 || r->stage == 2 || r->visible) rear = r->position-half;
            if ((stopped_ && r->stage == 2) || (reduced_ && r->stage == 3)) r->visible = false;
            if (std::abs(old-r->position)>0.01 || wasVisible != r->visible) {
                const auto i = index(static_cast<int>(r-rows_.data()),0);
                emit dataChanged(i,i,{Position,Visible,Stage});
            }
        }
    }
}
