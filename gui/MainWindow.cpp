#include "MainWindow.hpp"
#include "TrafficCanvas.hpp"
#include "Simulador.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QFile>
#include <QFrame>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QSaveFile>
#include <QSignalBlocker>
#include <QScrollArea>
#include <QSpinBox>
#include <QTextStream>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QStringList>

#include <exception>
#include <string>
#include <utility>

namespace {
QFrame* panel(QWidget* parent = nullptr) {
    auto* frame = new QFrame(parent);
    frame->setObjectName("panel");
    frame->setFrameShape(QFrame::NoFrame);
    return frame;
}

QLabel* textLabel(const QString& value, const QString& role = "small") {
    auto* label = new QLabel(value);
    label->setObjectName(role);
    label->setWordWrap(role == "small" || role == "subtitle" || role == "statTitle");
    return label;
}

QWidget* field(const QString& title, QSpinBox*& control, int lo, int hi,
               int value, const QString& suffix = {}) {
    auto* wrap = new QWidget;
    auto* lay = new QVBoxLayout(wrap);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(4);
    lay->addWidget(textLabel(title, "fieldLabel"));
    control = new QSpinBox;
    control->setRange(lo, hi);
    control->setValue(value);
    control->setSuffix(suffix);
    control->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
    control->setMinimumHeight(35);
    lay->addWidget(control);
    return wrap;
}

QFrame* statCard(const QString& name, QLabel*& value) {
    auto* card = new QFrame;
    card->setObjectName("statCard");
    auto* l = new QVBoxLayout(card);
    l->setContentsMargins(13, 12, 13, 12);
    l->setSpacing(4);
    l->addWidget(textLabel(name, "statTitle"));
    value = textLabel("0", "statValue");
    l->addWidget(value);
    return card;
}

QString csvQuote(QString t) {
    t.replace('"', "\"\"");
    return "\"" + t + "\"";
}
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("SimTrrafic | Centro de control Qt 6");
    resize(1460, 870);
    setMinimumSize(1020, 710);
    buildUi();
    ticker_ = new QTimer(this);
    ticker_->setInterval(75);
    connect(ticker_, &QTimer::timeout, this, [this] { refreshStatus(); });
    ticker_->start();
    resetView();
}

MainWindow::~MainWindow() {
    ticker_->stop();
    if (simulator_) simulator_->solicitarDetencion();
    cleanupWorker();
}

void MainWindow::buildUi() {
    setStyleSheet(R"QSS(
        QMainWindow, QScrollArea, QWidget#root { background: #0b1525; color: #e8f3ff; }
        QLabel { color: #e8f3ff; font-family: 'DejaVu Sans'; }
        QLabel#brand { font-size: 23px; font-weight: 800; color: #effaff; }
        QLabel#subtitle { font-size: 11px; color: #94abc4; }
        QLabel#panelTitle { font-size: 15px; font-weight: 800; }
        QLabel#fieldLabel, QLabel#small, QLabel#statTitle { font-size: 11px; color: #9eb2cb; }
        QLabel#statValue { font-size: 24px; font-weight: 800; color: #eff9ff; }
        QLabel#phase { background: #193342; color: #64ebbb; border-radius: 8px;
                       padding: 9px; font-weight: 700; }
        QLabel#state { background: #1d3544; color: #7cecc6; border: 1px solid #2b5960;
                       border-radius: 14px; padding: 8px 13px; font-weight: 700; }
        QFrame#panel { background: #15243a; border: 1px solid #2b3f58; border-radius: 15px; }
        QFrame#statCard { background: #1b304c; border-radius: 10px;
                          border: 1px solid #334d69; }
        QSpinBox, QComboBox { background: #0f1e32; color: #e9f4ff;
                             border: 1px solid #415a76; border-radius: 8px;
                             padding: 6px 34px 6px 10px; selection-background-color: #247e82; }
        QSpinBox:focus, QComboBox:focus { border: 1px solid #43d3b5; }
        QSpinBox::up-button, QSpinBox::down-button {
            subcontrol-origin: border; width: 25px; background: #253b57;
            border-left: 1px solid #3d5570;
        }
        QSpinBox::up-button { subcontrol-position: top right;
                             border-top-right-radius: 7px; }
        QSpinBox::down-button { subcontrol-position: bottom right;
                               border-bottom-right-radius: 7px; }
        QComboBox::drop-down { border: 0; width: 27px; }
        QCheckBox { color: #e8f3ff; spacing: 8px; }
        QCheckBox::indicator { width: 16px; height: 16px; }
        QPushButton { background: #263e5d; color: #e9f3ff;
                      border: 1px solid #406083; border-radius: 9px;
                      padding: 9px; font-weight: 700; min-height: 21px; }
        QPushButton:hover { background: #33577b; }
        QPushButton:disabled { background: #1a2b3d; color: #6d8199;
                               border-color: #293d55; }
        QPushButton#primary { background: #2bcca1; color: #092c27; border: none; }
        QPushButton#primary:hover { background: #65eac0; }
        QPushButton#danger { background: #71445d; color: #fff0f4; border: none; }
        QPushButton#danger:hover { background: #96546b; }
        QPlainTextEdit { background: #0c192c; color: #c9dcef;
                         border: 1px solid #344b67; border-radius: 9px;
                         padding: 8px; font-size: 11px;
                         font-family: 'DejaVu Sans Mono'; }
        QProgressBar { background: #0d1e31; border: 1px solid #2e4962;
                       min-height: 17px; border-radius: 8px;
                       text-align: center; color: #effcff; }
        QProgressBar::chunk { background: #2ccea9; border-radius: 7px; }
        QScrollBar:vertical { background: #112037; width: 9px; }
        QScrollBar::handle:vertical { background: #4a6583; border-radius: 4px; }
    )QSS");

    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto* root = new QWidget;
    root->setObjectName("root");
    root->setMinimumWidth(1090);
    auto* outer = new QVBoxLayout(root);
    outer->setContentsMargins(17, 14, 17, 15);
    outer->setSpacing(11);

    auto* heading = new QHBoxLayout;
    auto* headingText = new QVBoxLayout;
    headingText->setSpacing(3);
    headingText->addWidget(textLabel("SIMTRRAFIC   /   CENTRO DE CONTROL", "brand"));
    headingText->addWidget(textLabel("Intersección inteligente · simulación concurrente · motor C++17 + Qt 6", "subtitle"));
    heading->addLayout(headingText, 1);
    state_ = textLabel("LISTO PARA INICIAR", "state");
    state_->setAlignment(Qt::AlignCenter);
    state_->setMinimumWidth(150);
    heading->addWidget(state_);
    outer->addLayout(heading);

    auto* columns = new QHBoxLayout;
    columns->setSpacing(12);
    outer->addLayout(columns, 1);

    // Configuracion, margen izquierdo.
    auto* settings = panel();
    settings->setFixedWidth(248);
    auto* left = new QVBoxLayout(settings);
    left->setContentsMargins(15, 15, 15, 15);
    left->setSpacing(9);
    left->addWidget(textLabel("Configuración", "panelTitle"));
    left->addWidget(textLabel("Escenario", "fieldLabel"));
    preset_ = new QComboBox;
    preset_->addItems({"Normal", "Tráfico ligero", "Hora punta", "Personalizado"});
    preset_->setMinimumHeight(36);
    left->addWidget(preset_);
    left->addWidget(field("Vehículos por eje", vehicles_, 1, 60, 8));
    left->addWidget(field("Intervalo de llegada", arrival_, 10, 5000, 180, " ms"));
    left->addWidget(field("Tiempo de cruce", crossing_, 20, 5000, 420, " ms"));
    left->addWidget(field("Semáforo verde", green_, 100, 10000, 1800, " ms"));
    left->addWidget(field("Ambos en rojo", allRed_, 20, 5000, 180, " ms"));
    adaptive_ = new QCheckBox("Semáforos adaptativos");
    left->addWidget(adaptive_);
    animate_ = new QCheckBox("Animación fluida (60 FPS)");
    animate_->setChecked(true);
    animate_->setToolTip("La animación interpola el movimiento entre eventos reales del motor.");
    left->addWidget(animate_);
    left->addSpacing(3);
    start_ = new QPushButton("▶  Iniciar simulación");
    start_->setObjectName("primary");
    left->addWidget(start_);
    stop_ = new QPushButton("■  Detener");
    stop_->setObjectName("danger");
    left->addWidget(stop_);
    reset_ = new QPushButton("↻  Reiniciar panel");
    left->addWidget(reset_);
    left->addStretch(1);
    left->addWidget(textLabel("La configuración se aplica en la próxima ejecución. Los coches siguen eventos reales.", "small"));
    columns->addWidget(settings);

    // Simulacion y eventos.
    auto* center = new QVBoxLayout;
    center->setSpacing(11);
    columns->addLayout(center, 1);
    auto* scenePanel = panel();
    auto* scene = new QVBoxLayout(scenePanel);
    scene->setContentsMargins(12, 12, 12, 12);
    scene->setSpacing(10);
    auto* sceneHead = new QHBoxLayout;
    sceneHead->addWidget(textLabel("Mapa de la intersección", "panelTitle"));
    sceneHead->addStretch();
    sceneHead->addWidget(textLabel("●  ANIMACIÓN EN VIVO", "small"));
    runtime_ = textLabel("00:00", "small");
    sceneHead->addWidget(runtime_);
    scene->addLayout(sceneHead);
    canvas_ = new TrafficCanvas;
    scene->addWidget(canvas_, 1);
    scene->addWidget(textLabel("N/S: de norte a sur    ·    E/O: de oeste a este    ·    Autos y buses a escala visual", "small"));
    phase_ = textLabel("Semáforos en rojo", "phase");
    phase_->setAlignment(Qt::AlignCenter);
    scene->addWidget(phase_);
    center->addWidget(scenePanel, 5);

    auto* logPanel = panel();
    auto* logLayout = new QVBoxLayout(logPanel);
    logLayout->setContentsMargins(14, 12, 14, 13);
    auto* logHead = new QHBoxLayout;
    logHead->addWidget(textLabel("Bitácora de eventos", "panelTitle"));
    logHead->addStretch();
    export_ = new QPushButton("Exportar CSV");
    logHead->addWidget(export_);
    logLayout->addLayout(logHead);
    events_ = new QPlainTextEdit;
    events_->setReadOnly(true);
    events_->setMinimumHeight(118);
    logLayout->addWidget(events_);
    center->addWidget(logPanel, 2);

    // Estadisticas, margen derecho.
    auto* statsPanel = panel();
    statsPanel->setFixedWidth(234);
    auto* right = new QVBoxLayout(statsPanel);
    right->setContentsMargins(12, 14, 12, 14);
    right->setSpacing(9);
    right->addWidget(textLabel("Estadísticas", "panelTitle"));
    right->addWidget(statCard("Vehículos generados", generated_));
    right->addWidget(statCard("Vehículos atendidos", served_));
    right->addWidget(statCard("Pendientes", pending_));
    right->addWidget(statCard("Espera media estimada", avgWait_));
    right->addSpacing(9);
    right->addWidget(textLabel("EJE NORTE–SUR", "fieldLabel"));
    nsCount_ = textLabel("0 / 0", "small");
    right->addWidget(nsCount_);
    nsProgress_ = new QProgressBar;
    right->addWidget(nsProgress_);
    right->addSpacing(9);
    right->addWidget(textLabel("EJE ESTE–OESTE", "fieldLabel"));
    eoCount_ = textLabel("0 / 0", "small");
    right->addWidget(eoCount_);
    eoProgress_ = new QProgressBar;
    right->addWidget(eoProgress_);
    right->addStretch(1);
    right->addWidget(textLabel("Vehículos animados con eventos del motor real. Las posiciones del mapa son ilustrativas.", "small"));
    columns->addWidget(statsPanel);

    scroll->setWidget(root);
    setCentralWidget(scroll);
    connect(preset_, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int value) { applyPreset(value); });
    connect(start_, &QPushButton::clicked, this, [this] { startSimulation(); });
    connect(stop_, &QPushButton::clicked, this, [this] { stopSimulation(); });
    connect(reset_, &QPushButton::clicked, this, [this] { resetView(); });
    connect(export_, &QPushButton::clicked, this, [this] { exportCsv(); });
    connect(animate_, &QCheckBox::toggled, this,
            [this](bool checked) { canvas_->setAnimationsEnabled(checked); });
    auto personalizado = [this] {
        if (preset_->currentIndex() == 3) return;
        const QSignalBlocker bloqueo(preset_);
        preset_->setCurrentIndex(3);
    };
    for (QSpinBox* control : {vehicles_, arrival_, crossing_, green_, allRed_}) {
        connect(control, QOverload<int>::of(&QSpinBox::valueChanged), this,
                [personalizado](int) { personalizado(); });
    }
    connect(adaptive_, &QCheckBox::toggled, this,
            [personalizado](bool) { personalizado(); });
}

void MainWindow::applyPreset(int index) {
    if (index == 3) return;
    const QSignalBlocker b1(vehicles_);
    const QSignalBlocker b2(arrival_);
    const QSignalBlocker b3(crossing_);
    const QSignalBlocker b4(green_);
    const QSignalBlocker b5(allRed_);
    const QSignalBlocker b6(adaptive_);
    ConfiguracionSimulacion c;
    if (index == 0) {
        // Escenario de demostracion: se distinguen claramente los cruces.
        c.vehiculosPorEje = 8;
        c.intervaloLlegadaMs = 180;
        c.tiempoCruceMs = 420;
        c.duracionVerdeMs = 1800;
        c.duracionTodoRojoMs = 180;
    } else if (index == 1) {
        c.vehiculosPorEje = 4;
        c.intervaloLlegadaMs = 230;
        c.tiempoCruceMs = 340;
        c.duracionVerdeMs = 1700;
        c.duracionTodoRojoMs = 180;
    } else if (index == 2) {
        c.vehiculosPorEje = 22;
        c.intervaloLlegadaMs = 65;
        c.tiempoCruceMs = 390;
        c.duracionVerdeMs = 2200;
        c.duracionTodoRojoMs = 200;
        c.adaptativo = true;
    }
    vehicles_->setValue(c.vehiculosPorEje);
    arrival_->setValue(c.intervaloLlegadaMs);
    crossing_->setValue(c.tiempoCruceMs);
    green_->setValue(c.duracionVerdeMs);
    allRed_->setValue(c.duracionTodoRojoMs);
    adaptive_->setChecked(c.adaptativo);
}

ConfiguracionSimulacion MainWindow::currentConfig() const {
    ConfiguracionSimulacion c;
    c.vehiculosPorEje = vehicles_->value();
    c.intervaloLlegadaMs = arrival_->value();
    c.tiempoCruceMs = crossing_->value();
    c.duracionVerdeMs = green_->value();
    c.duracionTodoRojoMs = allRed_->value();
    c.adaptativo = adaptive_->isChecked();
    c.mostrarConsola = false;
    c.validar();
    return c;
}

void MainWindow::setControlsEnabled(bool enabled) {
    preset_->setEnabled(enabled);
    vehicles_->setEnabled(enabled);
    arrival_->setEnabled(enabled);
    crossing_->setEnabled(enabled);
    green_->setEnabled(enabled);
    allRed_->setEnabled(enabled);
    adaptive_->setEnabled(enabled);
    start_->setEnabled(enabled);
    reset_->setEnabled(enabled);
}

void MainWindow::cleanupWorker() {
    if (worker_.joinable()) worker_.join();
}

void MainWindow::startSimulation() {
    if (monitor_ && monitor_->ejecutando()) return;
    cleanupWorker();
    try {
        const auto config = currentConfig();
        auto monitor = std::make_shared<MonitorSimulacion>();
        monitor->iniciar(config);
        auto stopping = std::make_shared<std::atomic<bool>>(false);
        std::weak_ptr<MonitorSimulacion> weakMonitor(monitor);
        auto simulator = std::make_shared<Simulador>(config,
            [weakMonitor](const EventoSimulacion& event,
                          const ResumenEstadisticas& summary,
                          const EstadoInterseccion& lights) {
                if (auto live = weakMonitor.lock()) live->registrar(event, summary, lights);
            });
        worker_ = std::thread([monitor, simulator, stopping] {
            try {
                const auto result = simulator->ejecutar();
                monitor->finalizar(result, stopping->load());
            } catch (const std::exception& e) {
                monitor->fallar(e.what());
            } catch (...) {
                monitor->fallar("Error desconocido del simulador");
            }
        });
        monitor_ = std::move(monitor);
        simulator_ = std::move(simulator);
        stopRequested_ = std::move(stopping);
        lastEvents_.clear();
        lastSnapshot_ = {};
        canvas_->resetAnimation();
        events_->setPlainText("Preparando los vehículos y los semáforos...");
        setControlsEnabled(false);
        stop_->setEnabled(true);
        state_->setText("EN EJECUCIÓN");
    } catch (const std::exception& error) {
        QMessageBox::critical(this, "No se pudo iniciar", QString::fromUtf8(error.what()));
    }
}

void MainWindow::stopSimulation() {
    if (!simulator_ || !monitor_ || !monitor_->ejecutando()) return;
    if (stopRequested_) stopRequested_->store(true);
    simulator_->solicitarDetencion();
    state_->setText("DETENIENDO…");
    stop_->setEnabled(false);
}

void MainWindow::resetView() {
    if (monitor_ && monitor_->ejecutando()) return;
    cleanupWorker();
    simulator_.reset();
    stopRequested_.reset();
    monitor_.reset();
    lastEvents_.clear();
    lastSnapshot_ = {};
    events_->setPlainText("La bitácora aparecerá aquí al iniciar una simulación.");
    generated_->setText("0");
    served_->setText("0");
    pending_->setText("0");
    avgWait_->setText("0 ms");
    runtime_->setText("00:00");
    nsCount_->setText("0 / 0");
    eoCount_->setText("0 / 0");
    nsProgress_->setRange(0, 1);
    eoProgress_->setRange(0, 1);
    nsProgress_->setValue(0);
    eoProgress_->setValue(0);
    phase_->setText("Semáforos en rojo");
    state_->setText("LISTO PARA INICIAR");
    canvas_->resetAnimation();
    setControlsEnabled(true);
    stop_->setEnabled(false);
    export_->setEnabled(false);
}

void MainWindow::refreshStatus() {
    if (!monitor_) return;
    const QByteArray bytes = QByteArray::fromStdString(monitor_->json());
    QJsonParseError error;
    const auto doc = QJsonDocument::fromJson(bytes, &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject()) return;
    const auto state = doc.object();
    renderSnapshot(state);
    if (!state.value("running").toBool()) {
        cleanupWorker();
        setControlsEnabled(true);
        stop_->setEnabled(false);
    }
}

void MainWindow::renderSnapshot(const QJsonObject& s) {
    lastSnapshot_ = s;
    const bool running = s.value("running").toBool();
    const bool finished = s.value("finished").toBool();
    const bool stopped = s.value("stopped").toBool();
    const int created = s.value("generated").toInt();
    const int served = s.value("served").toInt();
    const int pending = s.value("pending").toInt();
    generated_->setText(QString::number(created));
    served_->setText(QString::number(served));
    pending_->setText(QString::number(pending));
    avgWait_->setText(QString::number(s.value("avg_wait_ms").toDouble(), 'f', 0) + " ms");
    const auto duration = static_cast<qint64>(s.value("elapsed_ms").toDouble());
    runtime_->setText(QString("Tiempo %1:%2")
                          .arg(duration / 60000, 2, 10, QChar('0'))
                          .arg((duration / 1000) % 60, 2, 10, QChar('0')));
    const auto ns = s.value("ns").toObject();
    const auto eo = s.value("eo").toObject();
    const int nsMade = ns.value("generated").toInt();
    const int eoMade = eo.value("generated").toInt();
    const int nsDone = ns.value("served").toInt();
    const int eoDone = eo.value("served").toInt();
    nsCount_->setText(QString("%1 / %2 atendidos").arg(nsDone).arg(nsMade));
    eoCount_->setText(QString("%1 / %2 atendidos").arg(eoDone).arg(eoMade));
    nsProgress_->setMaximum(qMax(1, nsMade));
    eoProgress_->setMaximum(qMax(1, eoMade));
    nsProgress_->setValue(nsDone);
    eoProgress_->setValue(eoDone);
    const auto phase = s.value("phase").toString();
    const bool nsOn = phase == "ns";
    const bool eoOn = phase == "eo";
    phase_->setText(nsOn ? "NORTE–SUR EN VERDE  •  ESTE–OESTE EN ROJO"
                       : eoOn ? "ESTE–OESTE EN VERDE  •  NORTE–SUR EN ROJO"
                              : "FASE DE SEGURIDAD  •  AMBOS EN ROJO");
    canvas_->setSnapshot(s);
    const auto error = s.value("error").toString();
    if (!error.isEmpty()) state_->setText("ERROR: " + error);
    else if (running) state_->setText("EN EJECUCIÓN");
    else if (finished && stopped) state_->setText("DETENIDA");
    else if (finished) state_->setText("COMPLETADA");

    const auto events = s.value("events").toArray();
    const QByteArray current = QJsonDocument(events).toJson(QJsonDocument::Compact);
    if (current != lastEvents_) {
        lastEvents_ = current;
        QStringList lines;
        for (const auto& item : events) {
            const auto row = item.toObject();
            lines << QString("[%1 ms] %2")
                         .arg(row.value("ms").toInt(), 5)
                         .arg(row.value("text").toString());
        }
        events_->setPlainText(lines.isEmpty() ? "Sin eventos todavía" : lines.join('\n'));
    }
    export_->setEnabled(!events.isEmpty());
}

void MainWindow::exportCsv() {
    if (lastSnapshot_.isEmpty()) return;
    const auto events = lastSnapshot_.value("events").toArray();
    if (events.isEmpty()) return;
    const auto path = QFileDialog::getSaveFileName(this, "Exportar eventos CSV",
                                                    "simtrrafic_eventos.csv",
                                                    "Archivos CSV (*.csv)");
    if (path.isEmpty()) return;
    QSaveFile output(path);
    if (!output.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "No se pudo abrir el archivo seleccionado.");
        return;
    }
    QTextStream out(&output);
    out << "tipo,eje,id,tiempo_ms,descripcion\n";
    for (int i = events.size() - 1; i >= 0; --i) {
        const auto row = events.at(i).toObject();
        out << csvQuote(row.value("type").toString()) << ','
            << csvQuote(row.value("axis").toString()) << ','
            << row.value("id").toInt() << ','
            << row.value("ms").toInt() << ','
            << csvQuote(row.value("text").toString()) << '\n';
    }
    out.flush();
    if (out.status() != QTextStream::Ok || !output.commit()) {
        QMessageBox::warning(this, "Error", "No se pudieron guardar los eventos.");
    } else {
        QMessageBox::information(this, "Exportado", "Eventos guardados en CSV.");
    }
}
