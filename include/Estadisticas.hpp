#pragma once

#include "Interseccion.hpp"

#include <atomic>
#include <cstdint>

// Resumen que se consulta al terminar la simulacion.
struct ResumenEstadisticas {
    int generadosNS;
    int generadosEO;
    int atendidosNS;
    int atendidosEO;
    std::int64_t esperaNSms = 0;
    std::int64_t esperaEOms = 0;

    double esperaPromedioNS() const { return atendidosNS ? double(esperaNSms) / atendidosNS : 0.0; }
    double esperaPromedioEO() const { return atendidosEO ? double(esperaEOms) / atendidosEO : 0.0; }
    double esperaPromedio() const {
        return totalAtendidos() ? double(esperaNSms + esperaEOms) / totalAtendidos() : 0.0;
    }

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

    void registrarEspera(Eje eje, std::int64_t milisegundos) {
        if (eje == Eje::NorteSur) esperaNSms_.fetch_add(milisegundos, std::memory_order_relaxed);
        else esperaEOms_.fetch_add(milisegundos, std::memory_order_relaxed);
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
            atendidosEO_.load(std::memory_order_relaxed),
            esperaNSms_.load(std::memory_order_relaxed),
            esperaEOms_.load(std::memory_order_relaxed)
        };
    }

private:
    std::atomic<int> generadosNS_{0};
    std::atomic<int> generadosEO_{0};
    std::atomic<int> atendidosNS_{0};
    std::atomic<int> atendidosEO_{0};
    std::atomic<std::int64_t> esperaNSms_{0};
    std::atomic<std::int64_t> esperaEOms_{0};
};
