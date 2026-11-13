#pragma once

#include "Interseccion.hpp"

#include <atomic>

// Resumen que se consulta al terminar la simulacion.
struct ResumenEstadisticas {
    int generadosNS;
    int generadosEO;
    int atendidosNS;
    int atendidosEO;

    int totalGenerados() const { return generadosNS + generadosEO; }
    int totalAtendidos() const { return atendidosNS + atendidosEO; }
    int pendientes() const { return totalGenerados() - totalAtendidos(); }
};

// Contadores seguros para ser actualizados por varios hilos.
class Estadisticas {
public:
    void registrarGenerado(Eje eje) {
        if (eje == Eje::NorteSur) {
            generadosNS_.fetch_add(1, std::memory_order_relaxed);
        } else {
            generadosEO_.fetch_add(1, std::memory_order_relaxed);
        }
    }

    void registrarAtendido(Eje eje) {
        if (eje == Eje::NorteSur) {
            atendidosNS_.fetch_add(1, std::memory_order_relaxed);
        } else {
            atendidosEO_.fetch_add(1, std::memory_order_relaxed);
        }
    }

    int totalAtendidos() const {
        return atendidosNS_.load(std::memory_order_relaxed) +
               atendidosEO_.load(std::memory_order_relaxed);
    }

    // Leer este resumen una vez que hayan terminado generadores y despachadores.
    ResumenEstadisticas resumenFinal() const {
        return {
            generadosNS_.load(std::memory_order_relaxed),
            generadosEO_.load(std::memory_order_relaxed),
            atendidosNS_.load(std::memory_order_relaxed),
            atendidosEO_.load(std::memory_order_relaxed)
        };
    }

private:
    std::atomic<int> generadosNS_{0};
    std::atomic<int> generadosEO_{0};
    std::atomic<int> atendidosNS_{0};
    std::atomic<int> atendidosEO_{0};
};
