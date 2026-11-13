#include "ColaConcurrente.hpp"
#include "Estadisticas.hpp"
#include "Interseccion.hpp"
#include "Vehiculo.hpp"

#include <chrono>
#include <functional>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

using namespace std::chrono_literals;

const char* nombreEje(Eje eje) {
    return eje == Eje::NorteSur ? "Norte-Sur" : "Este-Oeste";
}

int main() {
    constexpr int vehiculosPorEje = 6;
    constexpr int totalVehiculos = 2 * vehiculosPorEje;

    ColaConcurrente<Vehiculo> carrilNS;
    ColaConcurrente<Vehiculo> carrilEO;
    Interseccion interseccion;
    Estadisticas estadisticas;
    std::mutex mutexSalida;

    auto mostrar = [&](const std::string& mensaje) {
        std::lock_guard<std::mutex> bloqueo(mutexSalida);
        std::cout << mensaje << '\n';
    };

    mostrar("SIM-TRAFIC | Avance 03 | Estadisticas de trafico");

    auto generar = [&](ColaConcurrente<Vehiculo>& carril, Eje eje, int idInicial) {
        for (int i = 0; i < vehiculosPorEje; ++i) {
            const int id = idInicial + i;
            Vehiculo vehiculo{id, (id % 3 == 0) ? "Bus" : "Auto"};
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

    std::thread generadorNS(generar, std::ref(carrilNS), Eje::NorteSur, 1);
    std::thread generadorEO(generar, std::ref(carrilEO), Eje::EsteOeste,
                           vehiculosPorEje + 1);

    std::thread controlador([&] {
        while (estadisticas.totalAtendidos() < totalVehiculos) {
            // El intervalo con ambas luces rojas permite despejar el cruce.
            interseccion.detener();
            mostrar("[SEMAFOROS] Ambos ejes en ROJO");
            std::this_thread::sleep_for(100ms);

            interseccion.habilitar(Eje::NorteSur);
            mostrar("[SEMAFOROS] Norte-Sur VERDE | Este-Oeste ROJO");
            std::this_thread::sleep_for(320ms);

            interseccion.detener();
            mostrar("[SEMAFOROS] Ambos ejes en ROJO");
            std::this_thread::sleep_for(100ms);

            interseccion.habilitar(Eje::EsteOeste);
            mostrar("[SEMAFOROS] Norte-Sur ROJO | Este-Oeste VERDE");
            std::this_thread::sleep_for(320ms);
        }
        interseccion.detener();
    });

    auto despachar = [&](ColaConcurrente<Vehiculo>& carril, Eje eje) {
        while (auto siguiente = carril.extraer()) {
            interseccion.esperarPaso(eje);
            mostrar("[PASO] " + std::string(nombreEje(eje)) +
                    " | Vehiculo " + std::to_string(siguiente->id) + " cruza");
            std::this_thread::sleep_for(75ms);
            estadisticas.registrarAtendido(eje);
        }
    };

    std::thread despachadorNS(despachar, std::ref(carrilNS), Eje::NorteSur);
    std::thread despachadorEO(despachar, std::ref(carrilEO), Eje::EsteOeste);

    generadorNS.join();
    generadorEO.join();
    despachadorNS.join();
    despachadorEO.join();
    controlador.join();

    const ResumenEstadisticas resumen = estadisticas.resumenFinal();
    mostrar("\n========== RESUMEN DE SIMULACION ==========");
    mostrar("Vehiculos generados: " + std::to_string(resumen.totalGenerados()));
    mostrar("Vehiculos atendidos: " + std::to_string(resumen.totalAtendidos()));
    mostrar("Norte-Sur: " + std::to_string(resumen.atendidosNS) +
            " atendidos de " + std::to_string(resumen.generadosNS) + " generados");
    mostrar("Este-Oeste: " + std::to_string(resumen.atendidosEO) +
            " atendidos de " + std::to_string(resumen.generadosEO) + " generados");
    mostrar("Vehiculos pendientes: " + std::to_string(resumen.pendientes()));
    mostrar("===========================================");

    return 0;
}
