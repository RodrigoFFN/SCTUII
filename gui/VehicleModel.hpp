#pragma once
#include "MonitorSimulacion.hpp"
#include <QAbstractListModel>
#include <QVariant>
#include <QString>
#include <vector>

// Solo posiciones de dibujo. Las etapas y los tiempos pertenecen al motor.
class VehicleModel final : public QAbstractListModel {
public:
    enum Role { Id = Qt::UserRole + 1, Axis, Bus, Position, Visible, Stage };
    explicit VehicleModel(QObject* parent = nullptr);
    int rowCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    void sync(const MonitorSimulacion::Instantanea& snapshot);
    void frame(double dt, double elapsed);
    void setExtent(double horizontal, double vertical);
    void clear();
    void setReducedMotion(bool reduced) { reduced_ = reduced; }
    int hiddenNS() const { return hiddenNS_; }
    int hiddenEO() const { return hiddenEO_; }
private:
    struct Row {
        int id = 0, stage = 0, arrival = -1, cross = -1, served = -1;
        bool ns = true, bus = false, visible = false, fresh = true;
        double position = 0, queue = 0, crossStart = 0;
        double halfLength() const { return bus ? 30 : 22; }
    };
    std::vector<Row> rows_;
    unsigned long long run_ = 0;
    int crossingMs_ = 130, hiddenNS_ = 0, hiddenEO_ = 0;
    double horizontal_ = 450, vertical_ = 300;
    bool stopped_ = false, reduced_ = false;
};
