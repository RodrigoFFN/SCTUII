#include "TrafficCanvas.hpp"

#include <QPainter>
#include <QPaintEvent>
#include <QPen>
#include <QFont>
#include <QLinearGradient>
#include <QString>
#include <algorithm>

namespace {
const QColor bg("#111d30");
const QColor road("#26374c");
const QColor lane("#62768a");
const QColor green("#3cdda5");
const QColor red("#f56c83");
const QColor amber("#ffcb70");

void light(QPainter& p, const QPointF& pt, bool on, const QString& label) {
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#071321"));
    p.drawRoundedRect(QRectF(pt.x() - 20, pt.y() - 23, 40, 65), 11, 11);
    p.setBrush(on ? green : QColor("#355e53"));
    p.drawEllipse(QPointF(pt.x(), pt.y() - 5), 10, 10);
    p.setBrush(on ? QColor("#733b4b") : red);
    p.drawEllipse(QPointF(pt.x(), pt.y() + 22), 10, 10);
    p.setPen(QColor("#d7e5f6"));
    QFont f = p.font();
    f.setPointSizeF(9);
    f.setBold(true);
    p.setFont(f);
    p.drawText(QRectF(pt.x() - 34, pt.y() + 46, 68, 22), Qt::AlignCenter, label);
}

void vehicle(QPainter& p, const QRectF& rect, QColor color, bool vertical) {
    p.save();
    p.translate(rect.center());
    if (vertical) p.rotate(90);
    const QRectF body(-rect.width()/2., -rect.height()/2., rect.width(), rect.height());
    p.setPen(Qt::NoPen);
    p.setBrush(color);
    p.drawRoundedRect(body, 5, 5);
    p.setBrush(QColor("#b7dcf0"));
    p.drawRoundedRect(QRectF(body.left()+8, body.top()+3, 12, body.height()-6), 2, 2);
    p.restore();
}
}

TrafficCanvas::TrafficCanvas(QWidget* parent) : QWidget(parent) {
    setMinimumSize(470, 440);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void TrafficCanvas::setTraffic(bool ns, bool eo, bool occupied, int nsWaiting, int eoWaiting) {
    northSouthGreen_ = ns;
    eastWestGreen_ = eo;
    occupied_ = occupied;
    northSouthWaiting_ = std::max(0, nsWaiting);
    eastWestWaiting_ = std::max(0, eoWaiting);
    update();
}

void TrafficCanvas::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), bg);
    const qreal sx = width() / 620.0;
    const qreal sy = height() / 470.0;
    p.scale(sx, sy);

    // Parcela y espacios verdes.
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#203d40"));
    p.drawRoundedRect(QRectF(26, 20, 568, 430), 18, 18);
    p.setBrush(road);
    p.drawRect(QRectF(244, 20, 132, 430));
    p.drawRect(QRectF(26, 170, 568, 132));
    p.setPen(QPen(lane, 2, Qt::DashLine));
    p.drawLine(QPointF(310, 22), QPointF(310, 169));
    p.drawLine(QPointF(310, 302), QPointF(310, 449));
    p.drawLine(QPointF(26, 236), QPointF(244, 236));
    p.drawLine(QPointF(376, 236), QPointF(594, 236));

    // Pasos peatonales: elementos visuales sin simulacion de peatones.
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(210, 226, 245, 145));
    for (int i = 0; i < 6; ++i) {
        p.drawRoundedRect(QRectF(251 + i * 21, 145, 12, 12), 2, 2);
        p.drawRoundedRect(QRectF(251 + i * 21, 315, 12, 12), 2, 2);
        p.drawRoundedRect(QRectF(216, 176 + i * 21, 12, 12), 2, 2);
        p.drawRoundedRect(QRectF(393, 176 + i * 21, 12, 12), 2, 2);
    }

    // Contadores de cola representados por vehiculos en la aproximacion.
    for (int i = 0; i < std::min(4, northSouthWaiting_); ++i) {
        vehicle(p, QRectF(278, 66 + i*24, 30, 17), QColor("#68b9f5"), true);
    }
    for (int i = 0; i < std::min(4, eastWestWaiting_); ++i) {
        vehicle(p, QRectF(62 + i*34, 263, 30, 17), QColor("#fcbf72"), false);
    }
    if (occupied_) {
        const bool vertical = northSouthGreen_ || !eastWestGreen_;
        vehicle(p, vertical ? QRectF(296, 226, 30, 17) : QRectF(293, 226, 30, 17),
                QColor("#f0f7ff"), vertical);
        p.setPen(QPen(amber, 2));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(QPointF(310, 236), 33, 33);
    }

    light(p, QPointF(402, 91), northSouthGreen_, "NORTE / SUR");
    light(p, QPointF(93, 335), eastWestGreen_, "ESTE / OESTE");

    p.setPen(QColor("#cbd8e6"));
    QFont f = p.font();
    f.setPointSize(10);
    f.setBold(true);
    p.setFont(f);
    p.drawText(QRectF(405, 30, 180, 32), Qt::AlignRight, "SIMULACION EN VIVO");
    f.setBold(false);
    p.setFont(f);
    p.drawText(QRectF(403, 390, 183, 34), Qt::AlignRight,
               QString("Cola N/S: %1   |   E/O: %2")
                   .arg(northSouthWaiting_).arg(eastWestWaiting_));
    p.setPen(QColor("#92a9c2"));
    p.drawText(QRectF(32, 29, 180, 25), Qt::AlignLeft, "CRUCE SIMPLIFICADO");
}
