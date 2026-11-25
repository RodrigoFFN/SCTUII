#pragma once
#include "MonitorSimulacion.hpp"
#include "Simulador.hpp"
#include "VehicleModel.hpp"
#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <QVariantMap>
#include <QVariantList>
#include <QUrl>
#include <memory>
#include <thread>
#include <atomic>

// Bitacora incremental: inserta los eventos nuevos sin recrear la lista visual.
class EventLogModel final : public QAbstractListModel {
public:
    enum Role { Time=Qt::UserRole+1, Type, Axis, Id, Description };
    int rowCount(const QModelIndex& parent={}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int,QByteArray> roleNames() const override;
    void append(const std::vector<EventoSimulacion>& events);
    void clear();
private:
    std::vector<EventoSimulacion> events_;
};

class SimulationController final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantMap status READ status NOTIFY statusChanged)
    Q_PROPERTY(QAbstractItemModel* events READ events CONSTANT)
    Q_PROPERTY(int eventCount READ eventCount NOTIFY eventsChanged)
    Q_PROPERTY(QVariantList history READ history NOTIFY historyChanged)
    Q_PROPERTY(QAbstractItemModel* vehicles READ vehicles CONSTANT)
    Q_PROPERTY(bool stopping READ stopping NOTIFY statusChanged)
    Q_PROPERTY(int hiddenNS READ hiddenNS NOTIFY queuesChanged)
    Q_PROPERTY(int hiddenEO READ hiddenEO NOTIFY queuesChanged)
    Q_PROPERTY(QString message READ message NOTIFY messageChanged)
    Q_PROPERTY(bool messageIsError READ messageIsError NOTIFY messageChanged)
public:
    explicit SimulationController(QObject* parent = nullptr);
    ~SimulationController() override;
    QVariantMap status() const { return status_; }
    QAbstractItemModel* events() { return &eventModel_; }
    int eventCount() const { return eventModel_.rowCount(); }
    QVariantList history() const { return history_; }
    QAbstractItemModel* vehicles() { return &vehicles_; }
    bool stopping() const { return stopping_; }
    int hiddenNS() const { return vehicles_.hiddenNS(); }
    int hiddenEO() const { return vehicles_.hiddenEO(); }
    QString message() const { return message_; }
    bool messageIsError() const { return messageIsError_; }
    Q_INVOKABLE bool start(int count, int arrival, int crossing, int green, int red, bool adaptive);
    Q_INVOKABLE void stop();
    Q_INVOKABLE void reset();
    Q_INVOKABLE void setReducedMotion(bool reduced);
    Q_INVOKABLE void setSceneExtent(double horizontal, double vertical);
    Q_INVOKABLE bool exportCsv(const QUrl& file);
    Q_INVOKABLE void clearMessage();
signals:
    void statusChanged();
    void eventsChanged();
    void historyChanged();
    void queuesChanged();
    void messageChanged();
private:
    void poll();
    void animate();
    void setMessage(const QString& text, bool error = false);
    void join();
    MonitorSimulacion monitor_;
    VehicleModel vehicles_;
    EventLogModel eventModel_;
    std::shared_ptr<Simulador> simulator_;
    std::thread worker_;
    std::atomic<bool> cancel_{false};
    bool stopping_ = false;
    QTimer pollTimer_, frameTimer_;
    QElapsedTimer clock_, snapshotClock_;
    qint64 previousFrame_ = 0;
    MonitorSimulacion::Instantanea snapshot_;
    QVariantMap status_;
    QVariantList history_;
    QString message_;
    bool messageIsError_ = false;
    unsigned long long run_ = 0;
    size_t eventCount_ = 0;
};
