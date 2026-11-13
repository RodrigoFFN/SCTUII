#pragma once

#include "Semaforo.hpp"

#include <mutex>

// Cada eje representa un conjunto de carriles que comparten una fase verde.
enum class Eje { NorteSur, EsteOeste };

struct EstadoInterseccion {
    Luz norteSur;
    Luz esteOeste;
};

class Interseccion {
public:
    // Apaga ambos ejes antes de habilitar uno para evitar verdes simultaneos.
    void habilitar(Eje eje) {
        std::lock_guard<std::mutex> bloqueo(mutex_);
        norteSur_.cambiarA(Luz::Rojo);
        esteOeste_.cambiarA(Luz::Rojo);
        if (eje == Eje::NorteSur) {
            norteSur_.cambiarA(Luz::Verde);
        } else {
            esteOeste_.cambiarA(Luz::Verde);
        }
    }

    void detener() {
        std::lock_guard<std::mutex> bloqueo(mutex_);
        norteSur_.cambiarA(Luz::Rojo);
        esteOeste_.cambiarA(Luz::Rojo);
    }

    // Un vehiculo puede comenzar a cruzar solo despues de observar luz verde.
    void esperarPaso(Eje eje) {
        if (eje == Eje::NorteSur) {
            norteSur_.esperarVerde();
        } else {
            esteOeste_.esperarVerde();
        }
    }

    // Lectura consistente de las dos luces para las pruebas y la futura GUI.
    EstadoInterseccion estado() const {
        std::lock_guard<std::mutex> bloqueo(mutex_);
        return {norteSur_.actual(), esteOeste_.actual()};
    }

private:
    mutable std::mutex mutex_;
    Semaforo norteSur_;
    Semaforo esteOeste_;
};
