#pragma once

#include "Semaforo.hpp"

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <functional>
#include <thread>

// Cada eje representa el sentido prioritario de una interseccion de dos vias.
enum class Eje { NorteSur, EsteOeste };

struct EstadoInterseccion {
    Luz norteSur;
    Luz esteOeste;
    bool ocupado = false;
};

class Interseccion {
public:
    // Primero cierra ambos ejes y espera a que termine el cruce anterior.
    void habilitar(Eje eje) {
        std::unique_lock<std::mutex> bloqueo(mutex_);
        ponerAmbosEnRojo();
        disponible_.wait(bloqueo, [this] { return !ocupado_ || cancelado_; });
        if (!cancelado_) {
            if (eje == Eje::NorteSur) norteSur_.cambiarA(Luz::Verde);
            else esteOeste_.cambiarA(Luz::Verde);
        }
        bloqueo.unlock();
        disponible_.notify_all();
    }

    void detener() {
        std::unique_lock<std::mutex> bloqueo(mutex_);
        ponerAmbosEnRojo();
        disponible_.wait(bloqueo, [this] { return !ocupado_; });
        bloqueo.unlock();
        disponible_.notify_all();
    }

    // Conserva la API del prototipo original (pruebas antiguas).
    void esperarPaso(Eje eje) {
        if (eje == Eje::NorteSur) norteSur_.esperarVerde();
        else esteOeste_.esperarVerde();
    }

    // Reserva atomica de la zona critica: solo un vehiculo cruza a la vez.
    // La transicion de fase espera a que este cruce finalice.
    bool cruzar(Eje eje, std::chrono::milliseconds duracion,
                const std::function<void()>& alIniciar = {}) {
        std::unique_lock<std::mutex> bloqueo(mutex_);
        disponible_.wait(bloqueo, [this, eje] {
            return cancelado_ || (!ocupado_ && luzDe(eje) == Luz::Verde);
        });
        if (cancelado_) return false;
        ocupado_ = true;
        bloqueo.unlock();
        try {
            // El observador se ejecuta sin el mutex de la interseccion.
            if (alIniciar) alIniciar();
            std::this_thread::sleep_for(duracion);
        } catch (...) {
            bloqueo.lock();
            ocupado_ = false;
            bloqueo.unlock();
            disponible_.notify_all();
            throw;
        }
        bloqueo.lock();
        ocupado_ = false;
        bloqueo.unlock();
        disponible_.notify_all();
        return true;
    }

    void cancelar() {
        {
            std::lock_guard<std::mutex> bloqueo(mutex_);
            cancelado_ = true;
            ponerAmbosEnRojo();
        }
        disponible_.notify_all();
    }

    EstadoInterseccion estado() const {
        std::lock_guard<std::mutex> bloqueo(mutex_);
        return {norteSur_.actual(), esteOeste_.actual(), ocupado_};
    }

private:
    Luz luzDe(Eje eje) const {
        return eje == Eje::NorteSur ? norteSur_.actual() : esteOeste_.actual();
    }
    void ponerAmbosEnRojo() {
        norteSur_.cambiarA(Luz::Rojo);
        esteOeste_.cambiarA(Luz::Rojo);
    }
    mutable std::mutex mutex_;
    std::condition_variable disponible_;
    Semaforo norteSur_;
    Semaforo esteOeste_;
    bool ocupado_ = false;
    bool cancelado_ = false;
};
