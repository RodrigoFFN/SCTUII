#pragma once

#include "TrafficAnimationModel.hpp"

#include <QElapsedTimer>
#include <QHash>
#include <QJsonObject>
#include <QPointF>
#include <QTimer>
#include <QWidget>

// Animacion 2D esquematica: los eventos son reales y las coordenadas son
// interpolaciones visuales. No calcula ni modifica la simulacion de trafico.
class TrafficCanvas final : public QWidget {
public:
    explicit TrafficCanvas(QWidget* parent = nullptr);

    void setSnapshot(const QJsonObject& state);
    void resetAnimation();
    void setAnimationsEnabled(bool enabled);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    qreal currentTimeMs() const;

    TrafficAnimationModel model_;
    QHash<int, QPointF> smoothPositions_;
    QTimer animationTimer_;
    QElapsedTimer clock_;
    qint64 sampleElapsedMs_ = 0;
    qint64 sampleClockMs_ = 0;
    int crossingMs_ = 130;
    bool northSouthGreen_ = false;
    bool eastWestGreen_ = false;
    bool occupied_ = false;
    bool running_ = false;
    bool animate_ = true;
    int northSouthWaiting_ = 0;
    int eastWestWaiting_ = 0;
};
