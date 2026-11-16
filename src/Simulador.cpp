#include "Simulador.hpp"

#include "Carril.hpp"
#include "Interseccion.hpp"
#include "Vehiculo.hpp"

#include <chrono>
#include <exception>
#include <future>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace {

using namespace std::chrono_literals;

constexpr int vehiculosPorEje = 6;
constexpr int totalVehiculos = 2 * vehiculosPorEje;

const char* nombreEje(Eje eje) {
    return eje == Eje::NorteSur ? "Norte-Sur" : "Este-Oeste";
}

void mostrarResumen(const ResumenEstadisticas& resumen) {
    std::cout << "\n========== RESUMEN DE SIMULACION ==========\n"
              << "Vehiculos generados: " << resumen.totalGenerados() << '\n'
              << "Vehiculos atendidos: " << resumen.totalAtendidos() << '\n'
              << "Norte-Sur: " << resumen.atendidosNS << " atendidos de "
              << resumen.generadosNS << " generados\n"
              << "Este-Oeste: " << resumen.atendidosEO << " atendidos de "
              << resumen.generadosEO << " generados\n"
              << "Vehiculos pendientes: " << resumen.pendientes() << '\n'
              << "===========================================\n";
}

}  // namespace

ResumenEstadisticas Simulador::ejecutar() const {
    Carril carrilNS(Eje::NorteSur);
    Carril carrilEO(Eje::EsteOeste);
    Interseccion interseccion;
    Estadisticas estadisticas;
    std::mutex mutexSalida;

    auto mostrar = [&](const std::string& mensaje) {
        const std::lock_guard<std::mutex> bloqueo(mutexSalida);
        std::cout << mensaje << '\n';
    };

    mostrar("SIM-TRAFIC | Avance 03 | Estadisticas de trafico");

    auto generar = [&](Carril& carril, int idInicial) {
        const Eje eje = carril.eje();
        for (int i = 0; i < vehiculosPorEje; ++i) {
            const int id = idInicial + i;
            const Vehiculo vehiculo{id, (id % 3 == 0) ? "Bus" : "Auto"};
            if (carril.agregar(vehiculo)) {
                estadisticas.registrarGenerado(eje);
                mostrar("[LLEGADA] " + std::string(nombreEje(eje)) +
                        " | Vehiculo " + std::to_string(id) +
                        " (" + vehiculo.tipo + ")");
            }
            std::this_thread::sleep_for(60ms);
        }
        carril.cerrar();
    };

    auto despachar = [&](Carril& carril) {
        const Eje eje = carril.eje();
        while (auto vehiculo = carril.extraer()) {
            interseccion.esperarPaso(eje);
            mostrar("[PASO] " + std::string(nombreEje(eje)) +
                    " | Vehiculo " + std::to_string(vehiculo->id) + " cruza");
            std::this_thread::sleep_for(75ms);
            estadisticas.registrarAtendido(eje);
        }
    };

    auto fase = [&](Eje eje, const char* mensaje) {
        interseccion.detener();
        mostrar("[SEMAFOROS] Ambos ejes en ROJO");
        std::this_thread::sleep_for(100ms);

        {
            const std::lock_guard<std::mutex> bloqueo(mutexSalida);
            interseccion.habilitar(eje);
            std::cout << mensaje << '\n';
        }
        std::this_thread::sleep_for(320ms);
    };

    auto controlarSemaforos = [&] {
        while (estadisticas.totalAtendidos() < totalVehiculos) {
            fase(Eje::NorteSur, "[SEMAFOROS] Norte-Sur VERDE | Este-Oeste ROJO");
            fase(Eje::EsteOeste, "[SEMAFOROS] Norte-Sur ROJO | Este-Oeste VERDE");
        }
        interseccion.detener();
    };

    // Los cinco hilos esperan hasta que todos se hayan creado correctamente.
    std::promise<bool> iniciar;
    const auto listo = iniciar.get_future().share();
    std::vector<std::thread> hilos;
    hilos.reserve(5);

    auto lanzar = [&](auto tarea) {
        hilos.emplace_back([listo, tarea = std::move(tarea)] {
            if (listo.get()) {
                tarea();
            }
        });
    };

    std::exception_ptr errorAlIniciar;
    try {
        lanzar([&] { generar(carrilNS, 1); });
        lanzar([&] { generar(carrilEO, vehiculosPorEje + 1); });
        lanzar([&] { despachar(carrilNS); });
        lanzar([&] { despachar(carrilEO); });
        lanzar(controlarSemaforos);
    } catch (...) {
        errorAlIniciar = std::current_exception();
    }

    iniciar.set_value(!errorAlIniciar);
    for (auto& hilo : hilos) {
        hilo.join();
    }
    if (errorAlIniciar) {
        std::rethrow_exception(errorAlIniciar);
    }

    const auto resumen = estadisticas.resumenFinal();
    mostrarResumen(resumen);
    return resumen;
}
