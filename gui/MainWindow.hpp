#pragma once

#include "ConfiguracionSimulacion.hpp"
#include "MonitorSimulacion.hpp"

#include <QMainWindow>
#include <QJsonObject>
#include <QByteArray>
#include <memory>
#include <thread>
#include <atomic>

class QLabel;
class QProgressBar;
class QPushButton;
class QComboBox;
class QSpinBox;
class QCheckBox;
class QPlainTextEdit;
class QTimer;
class TrafficCanvas;
class Simulador;

class MainWindow final : public QMainWindow {
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private:
    void buildUi();
    void applyPreset(int index);
    ConfiguracionSimulacion currentConfig() const;
    void setControlsEnabled(bool enabled);
    void startSimulation();
    void stopSimulation();
    void resetView();
    void refreshStatus();
    void exportCsv();
    void renderSnapshot(const QJsonObject& state);
    void cleanupWorker();

    QComboBox* preset_ = nullptr;
    QSpinBox* vehicles_ = nullptr;
    QSpinBox* arrival_ = nullptr;
    QSpinBox* crossing_ = nullptr;
    QSpinBox* green_ = nullptr;
    QSpinBox* allRed_ = nullptr;
    QCheckBox* adaptive_ = nullptr;
    QCheckBox* animate_ = nullptr;
    QPushButton* start_ = nullptr;
    QPushButton* stop_ = nullptr;
    QPushButton* reset_ = nullptr;
    QPushButton* export_ = nullptr;
    QLabel* phase_ = nullptr;
    QLabel* state_ = nullptr;
    QLabel* generated_ = nullptr;
    QLabel* served_ = nullptr;
    QLabel* pending_ = nullptr;
    QLabel* avgWait_ = nullptr;
    QLabel* runtime_ = nullptr;
    QLabel* nsCount_ = nullptr;
    QLabel* eoCount_ = nullptr;
    QProgressBar* nsProgress_ = nullptr;
    QProgressBar* eoProgress_ = nullptr;
    QPlainTextEdit* events_ = nullptr;
    TrafficCanvas* canvas_ = nullptr;
    QTimer* ticker_ = nullptr;

    std::shared_ptr<MonitorSimulacion> monitor_;
    std::shared_ptr<Simulador> simulator_;
    std::shared_ptr<std::atomic<bool>> stopRequested_;
    std::thread worker_;
    QByteArray lastEvents_;
    QJsonObject lastSnapshot_;
};
