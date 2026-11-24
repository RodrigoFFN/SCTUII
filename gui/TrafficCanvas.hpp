#pragma once

#include <QWidget>

// Vista esquematica de una interseccion. Los coches en espera se representan
// como indicadores aproximados: la simulacion no incluye coordenadas fisicas.
class TrafficCanvas final : public QWidget {
public:
    explicit TrafficCanvas(QWidget* parent = nullptr);

    void setTraffic(bool northSouthGreen, bool eastWestGreen, bool occupied,
                    int northSouthWaiting, int eastWestWaiting);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    bool northSouthGreen_ = false;
    bool eastWestGreen_ = false;
    bool occupied_ = false;
    int northSouthWaiting_ = 0;
    int eastWestWaiting_ = 0;
};
