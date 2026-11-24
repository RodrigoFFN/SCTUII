#include "TrafficCanvas.hpp"

#include <QFont>
#include <QJsonArray>
#include <QJsonObject>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QPen>
#include <QString>

#include <algorithm>
#include <cmath>
#include <map>
#include <vector>

namespace {
constexpr qreal W = 820;
constexpr qreal H = 500;
const QColor ink("#d9e7f5");
const QColor secondary("#9db0c4");
const QColor road("#25394e");
const QColor border("#38526a");
const QColor grass("#173b37");
const QColor mint("#39dfae");
const QColor rose("#fb7185");
const QColor gold("#ffc979");

qreal bound(qreal v, qreal lo, qreal hi) {
    return std::clamp(v, lo, hi);
}
qreal easeOut(qreal t) {
    const qreal u = 1 - bound(t, 0, 1);
    return 1 - u * u * u;
}

void rounded(QPainter& p, QRectF r, QColor fill, qreal radius = 12) {
    p.setPen(Qt::NoPen);
    p.setBrush(fill);
    p.drawRoundedRect(r, radius, radius);
}

void pill(QPainter& p, QRectF r, const QString& label, QColor fill, QColor text) {
    rounded(p, r, fill, r.height() / 2);
    p.setPen(text);
    auto font = p.font();
    font.setPointSizeF(9);
    font.setWeight(QFont::DemiBold);
    p.setFont(font);
    p.drawText(r, Qt::AlignCenter, label);
}

void trafficLight(QPainter& p, QPointF center, bool greenOn, const QString& label) {
    rounded(p, QRectF(center.x()-16, center.y()-30, 32, 69), QColor("#081725"), 10);
    p.setPen(QPen(QColor("#294b58"), 1));
    p.setBrush(greenOn ? mint : QColor("#244e43"));
    p.drawEllipse(QPointF(center.x(), center.y()-11), 9, 9);
    p.setBrush(greenOn ? QColor("#653742") : rose);
    p.drawEllipse(QPointF(center.x(), center.y()+17), 9, 9);
    p.setPen(secondary);
    auto f = p.font(); f.setPointSizeF(9); f.setBold(true); p.setFont(f);
    p.drawText(QRectF(center.x()-53, center.y()+44, 106, 18), Qt::AlignCenter, label);
}

void car(QPainter& p, QPointF center, bool northSouth, int id, bool bus,
         bool crossing, qreal opacity = 1.0) {
    p.save();
    p.setOpacity(opacity);
    p.translate(center);
    if (northSouth) p.rotate(90);
    const qreal length = bus ? 73 : 53;
    const qreal height = bus ? 28 : 26;
    const QColor main = northSouth
        ? (bus ? QColor("#b1a0ff") : QColor("#54bffa"))
        : (bus ? QColor("#ffca84") : QColor("#ffc36d"));

    // Contact shadow plus tires, facing right in local coordinates.
    rounded(p, QRectF(-length/2+2, -height/2+4, length, height), QColor(0, 0, 0, 65), 7);
    p.setBrush(QColor("#0a1423"));
    p.setPen(Qt::NoPen);
    for (const qreal x : {-length * .29, length * .26}) {
        p.drawRoundedRect(QRectF(x-5, -height/2-3, 10, 6), 2, 2);
        p.drawRoundedRect(QRectF(x-5, height/2-3, 10, 6), 2, 2);
    }
    rounded(p, QRectF(-length/2, -height/2, length, height), main, 7);
    rounded(p, QRectF(-length*.18, -height*.37, length*.40, height*.74),
            QColor(bus ? "#36475a" : "#294766"), 5);
    p.setPen(QPen(QColor("#97e5ef"), 1.1));
    p.drawLine(QPointF(length*.21, -height*.29), QPointF(length*.21, height*.29));
    rounded(p, QRectF(length/2-5, -height/2+4, 3, height-8),
            crossing ? QColor("#ffffff") : QColor("#e6faff"), 1);
    rounded(p, QRectF(-length/2+2, -height/2+5, 3, height-10), rose, 1);
    p.setPen(QColor("#f4fbff"));
    auto f = p.font(); f.setPointSizeF(bus ? 8 : 9); f.setBold(true); p.setFont(f);
    p.drawText(QRectF(-length/2, -9, length, 18), Qt::AlignCenter, QString::number(id));
    p.restore();
}

void roadArrow(QPainter& p, QPointF center, bool vertical) {
    p.save();
    p.translate(center);
    if (vertical) p.rotate(90);
    p.setPen(QPen(QColor("#91a6bb"), 2.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.drawLine(QPointF(-18,0), QPointF(18,0));
    p.drawLine(QPointF(10,-7), QPointF(18,0));
    p.drawLine(QPointF(10,7), QPointF(18,0));
    p.restore();
}
}

TrafficCanvas::TrafficCanvas(QWidget* parent) : QWidget(parent) {
    setMinimumSize(420, 320);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    clock_.start();
    animationTimer_.setInterval(16);
    connect(&animationTimer_, &QTimer::timeout, this, [this] { update(); });
    animationTimer_.start();
    setToolTip("Los autos avanzan a partir de eventos reales de llegada y cruce; la escala es esquematica.");
}

void TrafficCanvas::resetAnimation() {
    model_.clear();
    smoothPositions_.clear();
    northSouthGreen_ = eastWestGreen_ = occupied_ = running_ = false;
    northSouthWaiting_ = eastWestWaiting_ = 0;
    sampleElapsedMs_ = 0;
    sampleClockMs_ = clock_.elapsed();
    update();
}

void TrafficCanvas::setAnimationsEnabled(bool enabled) {
    animate_ = enabled;
    if (enabled) animationTimer_.start(); else animationTimer_.stop();
    update();
}

void TrafficCanvas::setSnapshot(const QJsonObject& s) {
    const auto elapsed = static_cast<qint64>(s.value("elapsed_ms").toDouble());
    if (sampleElapsedMs_ != elapsed) {
        sampleElapsedMs_ = elapsed;
        sampleClockMs_ = clock_.elapsed();
    }
    running_ = s.value("running").toBool();
    const auto phase = s.value("phase").toString();
    northSouthGreen_ = phase == "ns";
    eastWestGreen_ = phase == "eo";
    occupied_ = s.value("occupied").toBool();
    northSouthWaiting_ = std::max(0, s.value("ns").toObject().value("generated").toInt()
                                   - s.value("ns").toObject().value("served").toInt());
    eastWestWaiting_ = std::max(0, s.value("eo").toObject().value("generated").toInt()
                                  - s.value("eo").toObject().value("served").toInt());
    crossingMs_ = std::max(20, s.value("config").toObject().value("crossing").toInt(130));

    // El monitor devuelve los mas recientes primero: procesar en orden temporal.
    const auto events = s.value("events").toArray();
    for (int i = events.size()-1; i >= 0; --i) {
        const auto event = events.at(i).toObject();
        const auto tipo = event.value("type").toString();
        if (tipo != "llegada" && tipo != "cruce_inicio" && tipo != "paso") continue;
        model_.registrar(tipo.toStdString(), event.value("axis").toString() == "ns",
                        event.value("id").toInt(), event.value("ms").toInt(), crossingMs_);
    }
    update();
}

qreal TrafficCanvas::currentTimeMs() const {
    return sampleElapsedMs_ + std::max<qint64>(0, clock_.elapsed() - sampleClockMs_);
}

void TrafficCanvas::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), QColor("#0c1729"));

    const qreal scale = std::min(width()/W, height()/H);
    const qreal dx = (width() - W*scale)/2.0;
    const qreal dy = (height() - H*scale)/2.0;
    p.translate(dx, dy);
    p.scale(scale, scale);

    // Parcelas urbanas y calzada con espacio suficiente para coches.
    rounded(p, QRectF(10,10,800,480), QColor("#173735"), 19);
    p.setBrush(QColor("#1b4941")); p.setPen(Qt::NoPen);
    for (const auto& r : {QRectF(22,22,225,145), QRectF(545,340,245,132),
                          QRectF(28,351,210,118), QRectF(555,24,230,138)}) {
        p.drawRoundedRect(r, 16, 16);
    }
    rounded(p, QRectF(321,10,178,480), road, 3);
    rounded(p, QRectF(10,175,800,160), road, 3);
    rounded(p, QRectF(321,175,178,160), QColor("#2c4053"), 1);
    p.setPen(QPen(QColor("#7890a9"), 2, Qt::DashLine));
    p.drawLine(QPointF(410,10), QPointF(410,168));
    p.drawLine(QPointF(410,342), QPointF(410,490));
    p.drawLine(QPointF(10,255), QPointF(316,255));
    p.drawLine(QPointF(506,255), QPointF(810,255));
    p.setPen(QPen(QColor("#d8e7f5"), 4));
    p.drawLine(QPointF(323,166), QPointF(409,166));
    p.drawLine(QPointF(315,256), QPointF(315,333));
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(217,237,246,164));
    for (int i = 0; i < 5; ++i) {
        p.drawRoundedRect(QRectF(327+i*34, 181, 23, 7), 2, 2);
        p.drawRoundedRect(QRectF(327+i*34, 324, 23, 7), 2, 2);
        p.drawRoundedRect(QRectF(306, 186+i*28, 7, 19), 2, 2);
        p.drawRoundedRect(QRectF(508, 186+i*28, 7, 19), 2, 2);
    }
    roadArrow(p, QPointF(373, 400), true);
    roadArrow(p, QPointF(653, 283), false);

    // Rotulos fuera del area de rodadura.
    pill(p, QRectF(27,26,200,26), "SIMTRRAFIC  /  CRUCE 01", QColor("#0e252a"), ink);
    pill(p, QRectF(580,26,193,26), "MODO EN VIVO  ·  2 EJES", QColor("#0e252a"), mint);
    pill(p, QRectF(27,444,218,27),
         QString("EN COLA   N/S %1   |   E/O %2").arg(northSouthWaiting_).arg(eastWestWaiting_),
         QColor("#112838"), ink);

    trafficLight(p, QPointF(550,99), northSouthGreen_, "NORTE / SUR");
    trafficLight(p, QPointF(253,373), eastWestGreen_, "ESTE / OESTE");

    // La posicion de cada coche parte de los instantes del motor; entre ellos
    // solo suavizamos el movimiento en la representacion, sin inventar pasos.
    const qreal t = currentTimeMs();
    auto cars = model_.visibles(static_cast<int>(t));
    std::map<int,int> ranksNS, ranksEO;
    std::vector<VehiculoAnimado> waitingNS, waitingEO;
    for (const auto& c : cars) {
        if (!c.enCola()) continue;
        (c.norteSur ? waitingNS : waitingEO).push_back(c);
    }
    auto order = [](const VehiculoAnimado& a, const VehiculoAnimado& b) {
        if (a.llegadaMs != b.llegadaMs) return a.llegadaMs < b.llegadaMs;
        return a.id < b.id;
    };
    std::sort(waitingNS.begin(), waitingNS.end(), order);
    std::sort(waitingEO.begin(), waitingEO.end(), order);
    for (int i=0; i < int(waitingNS.size()); ++i) ranksNS[waitingNS[i].id]=i;
    for (int i=0; i < int(waitingEO.size()); ++i) ranksEO[waitingEO[i].id]=i;

    for (const auto& c : cars) {
        const bool ns = c.norteSur;
        const int slot = ns ? ranksNS[c.id] : ranksEO[c.id];
        // Solo los coches cercanos al cruce estan dentro del recorte visible.
        if (c.enCola() && slot >= (ns ? 4 : 5)) continue;

        QPointF destination;
        bool crossing = false;
        qreal opacity = 1.0;
        if (c.enCola()) {
            const qreal progress = easeOut((t - c.llegadaMs) / 450.0);
            const qreal stop = ns ? 134 - slot*58 : 284 - slot*65;
            destination = ns
                ? QPointF(372, -50 + (stop + 50)*progress)
                : QPointF(-60 + (stop + 60)*progress, 287);
        } else if (c.salidaMs < 0) {
            crossing = true;
            const qreal progress = bound((t - c.entradaMs) / crossingMs_, 0, 1);
            destination = ns ? QPointF(372, 136 + progress*226)
                             : QPointF(284 + progress*262, 287);
        } else {
            const qreal elapsed = std::max<qreal>(0, t - c.salidaMs);
            const qreal progress = easeOut(elapsed / 690.0);
            destination = ns ? QPointF(372, 362 + progress*184)
                             : QPointF(546 + progress*300, 287);
            opacity = 1 - .85*bound((elapsed-450)/300., 0, 1);
        }

        // Evita el salto brusco de posiciones al avanzar la fila de espera.
        if (c.enCola()) {
            const auto prev = smoothPositions_.value(c.id, destination);
            const QPointF smooth(prev.x() + (destination.x()-prev.x())*.20,
                                 prev.y() + (destination.y()-prev.y())*.20);
            smoothPositions_.insert(c.id, smooth);
            destination = animate_ ? smooth : destination;
        } else {
            smoothPositions_.remove(c.id);
        }
        car(p, destination, ns, c.id, c.esBus(), crossing, opacity);
    }

    if (occupied_) {
        p.setPen(QPen(QColor(57,223,174,115), 2));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(QRectF(344,198,132,112), 15, 15);
    }
    if (model_.totalRegistrados() == 0) {
        pill(p, QRectF(308,230,206,44), "LISTO PARA INICIAR", QColor("#112638"), ink);
    }
}
