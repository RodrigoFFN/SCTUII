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
    label->setWordWrap(true);
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
    control->setButtonSymbols(QAbstractSpinBox::PlusMinus);
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
    resize(1450, 900);
    setMinimumSize(1060, 710);
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
        QMainWindow, QScrollArea, QWidget#root { background: #0c1525; color: #e5edfa; }
        QLabel { color: #e5edfa; font-family: 'DejaVu Sans'; }
        QLabel#brand { font-size: 26px; font-weight: 800; color: #f2f7ff; }
        QLabel#subtitle { font-size: 12px; color: #92a8be; }
        QLabel#panelTitle { font-size: 15px; font-weight: 700; }
        QLabel#fieldLabel, QLabel#small, QLabel#statTitle { font-size: 11px; color: #a2b3c8; }
        QLabel#statValue { font-size: 23px; font-weight: 800; color: #eaf3ff; }
        QLabel#phase { background: #182d3e; color: #63e4ae; border-radius: 9px; padding: 10px; font-weight: bold; }
        QLabel#state { background: #1b2840; color: #b2c9e5; border-radius: 9px; padding: 9px; font-weight: 700; }
        QFrame#panel { background: #152238; border: 1px solid #283a55; border-radius: 16px; }
        QFrame#statCard { background: #1c2d46; border-radius: 10px; border: 1px solid #2d4260; }
        QSpinBox, QComboBox { background: #0f1b2f; color: #e5edfa; border: 1px solid #3c5270; border-radius: 7px; padding: 5px 9px; }
        QComboBox::drop-down { border: 0; width: 20px; }
        QCheckBox { color: #e5edfa; spacing: 8px; }
        QCheckBox::indicator { width: 16px; height: 16px; }
        QPushButton { background: #273c59; color: #eaf3ff; border: 1px solid #385473; border-radius: 9px; padding: 10px; font-weight: 700; }
        QPushButton:hover { background: #375577; }
        QPushButton:disabled { background: #1b293a; color: #667b93; border-color: #24354a; }
        QPushButton#primary { background: #20b982; color: #071e1b; border: none; }
        QPushButton#primary:hover { background: #54dbab; }
        QPushButton#danger { background: #744157; color: #fff3f5; border: none; }
        QPlainTextEdit { background: #0d1a2d; color: #c9daf0; border: 1px solid #2e4360; border-radius: 10px; padding: 8px; font-size: 11px; }
        QProgressBar { background: #0d1a2d; border-radius: 7px; min-height: 17px; border: 1px solid #30415a; text-align: center; color: #e6effa; }
        QProgressBar::chunk { background: #29c59b; border-radius: 6px; }
        QScrollBar:vertical { background: #0f1b2e; width: 9px; }
        QScrollBar::handle:vertical { background: #47607e; border-radius: 4px; }
    )QSS");

    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto* root = new QWidget;
    root->setObjectName("root");
    root->setMinimumWidth(1140);
    auto* outer = new QVBoxLayout(root);
    outer->setContentsMargins(22, 21, 22, 22);
    outer->setSpacing(18);

    auto* heading = new QHBoxLayout;
    auto* headingText = new QVBoxLayout;
    headingText->setSpacing(3);
    headingText->addWidget(textLabel("SIMTRRAFIC  /  QT 6", "brand"));
    headingText->addWidget(textLabel("Panel de simulación concurrente de tráfico • escritorio", "subtitle"));
    heading->addLayout(headingText, 1);
    state_ = textLabel("LISTO PARA INICIAR", "state");
    state_->setAlignment(Qt::AlignCenter);
    state_->setMinimumWidth(160);
    heading->addWidget(state_);
    outer->addLayout(heading);

    auto* columns = new QHBoxLayout;
    columns->setSpacing(14);
    outer->addLayout(columns, 1);

    // Configuracion, margen izquierdo.
    auto* settings = panel();
    settings->setFixedWidth(260);
    auto* left = new QVBoxLayout(settings);
    left->setContentsMargins(16, 18, 16, 18);
    left->setSpacing(12);
    left->addWidget(textLabel("Configuración", "panelTitle"));
    left->addWidget(textLabel("Escenario", "fieldLabel"));
    preset_ = new QComboBox;
    preset_->addItems({"Normal", "Tráfico ligero", "Hora punta", "Personalizado"});
    preset_->setMinimumHeight(36);
    left->addWidget(preset_);
    left->addWidget(field("Vehículos por eje", vehicles_, 1, 60, 6));
    left->addWidget(field("Intervalo de llegada", arrival_, 10, 5000, 90, " ms"));
    left->addWidget(field("Tiempo de cruce", crossing_, 20, 5000, 130, " ms"));
    left->addWidget(field("Semáforo verde", green_, 100, 10000, 1000, " ms"));
    left->addWidget(field("Ambos en rojo", allRed_, 20, 5000, 220, " ms"));
    adaptive_ = new QCheckBox("Semáforos adaptativos");
    left->addWidget(adaptive_);
    left->addSpacing(6);
    start_ = new QPushButton("▶  Iniciar simulación");
    start_->setObjectName("primary");
    left->addWidget(start_);
    stop_ = new QPushButton("■  Detener");
    stop_->setObjectName("danger");
    left->addWidget(stop_);
    reset_ = new QPushButton("↻  Reiniciar panel");
    left->addWidget(reset_);
    left->addStretch(1);
    left->addWidget(textLabel("Los parámetros se aplican al iniciar una nueva ejecución.", "small"));
    columns->addWidget(settings);

    // Simulacion y eventos.
    auto* center = new QVBoxLayout;
    center->setSpacing(13);
    columns->addLayout(center, 1);
    auto* scenePanel = panel();
    auto* scene = new QVBoxLayout(scenePanel);
    scene->setContentsMargins(15, 14, 15, 14);
    scene->setSpacing(10);
    auto* sceneHead = new QHBoxLayout;
    sceneHead->addWidget(textLabel("Intersección N/S ↕ E/O", "panelTitle"));
    sceneHead->addStretch();
    runtime_ = textLabel("00:00", "small");
    sceneHead->addWidget(runtime_);
    scene->addLayout(sceneHead);
    canvas_ = new TrafficCanvas;
    scene->addWidget(canvas_, 1);
    phase_ = textLabel("Semáforos en rojo", "phase");
    phase_->setAlignment(Qt::AlignCenter);
    scene->addWidget(phase_);
    center->addWidget(scenePanel, 4);

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
    events_->setMinimumHeight(140);
    logLayout->addWidget(events_);
    center->addWidget(logPanel, 2);

    // Estadisticas, margen derecho.
    auto* statsPanel = panel();
    statsPanel->setFixedWidth(235);
    auto* right = new QVBoxLayout(statsPanel);
    right->setContentsMargins(13, 16, 13, 16);
    right->setSpacing(10);
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
    right->addWidget(textLabel("Los vehículos dibujados son esquemáticos; las cifras provienen del motor concurrente de C++.", "small"));
    columns->addWidget(statsPanel);

    scroll->setWidget(root);
    setCentralWidget(scroll);
    connect(preset_, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int value) { applyPreset(value); });
    connect(start_, &QPushButton::clicked, this, [this] { startSimulation(); });
    connect(stop_, &QPushButton::clicked, this, [this] { stopSimulation(); });
    connect(reset_, &QPushButton::clicked, this, [this] { resetView(); });
    connect(export_, &QPushButton::clicked, this, [this] { exportCsv(); });
}

void MainWindow::applyPreset(int index) {
    if (index == 3) return;
    ConfiguracionSimulacion c;
    if (index == 1) {
        c.vehiculosPorEje = 3;
        c.intervaloLlegadaMs = 250;
        c.tiempoCruceMs = 110;
        c.duracionVerdeMs = 1000;
        c.duracionTodoRojoMs = 220;
    } else if (index == 2) {
        c.vehiculosPorEje = 22;
        c.intervaloLlegadaMs = 35;
        c.tiempoCruceMs = 155;
        c.duracionVerdeMs = 1150;
        c.duracionTodoRojoMs = 240;
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
    canvas_->setTraffic(false, false, false, 0, 0);
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
    canvas_->setTraffic(nsOn, eoOn, s.value("occupied").toBool(),
                        nsMade - nsDone, eoMade - eoDone);
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
