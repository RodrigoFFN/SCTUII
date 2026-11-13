#include "ColaConcurrente.hpp"
#include "Interseccion.hpp"
#include "Vehiculo.hpp"

#include <atomic>
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
    std::atomic<int> atendidos{0};
    std::atomic<int> atendidosNS{0};
    std::atomic<int> atendidosEO{0};
    std::mutex mutexSalida;

    auto mostrar = [&](const std::string& mensaje) {
        std::lock_guard<std::mutex> bloqueo(mutexSalida);
        std::cout << mensaje << '\n';
    };

    mostrar("SIM-TRAFIC | Avance 02 | Interseccion de dos ejes");

    auto generar = [&](ColaConcurrente<Vehiculo>& carril, Eje eje, int idInicial) {
        for (int i = 0; i < vehiculosPorEje; ++i) {
            const int id = idInicial + i;
            Vehiculo vehiculo{id, (id % 3 == 0) ? "Bus" : "Auto"};
            mostrar("[LLEGADA] " + std::string(nombreEje(eje)) +
                    " | Vehiculo " + std::to_string(id) + " (" + vehiculo.tipo + ")");
            carril.agregar(vehiculo);
            std::this_thread::sleep_for(60ms);
        }
        carril.cerrar();
    };

    // Cada eje tiene su propio productor y consumidor.
    std::thread generadorNS(generar, std::ref(carrilNS), Eje::NorteSur, 1);
    std::thread generadorEO(generar, std::ref(carrilEO), Eje::EsteOeste,
                           vehiculosPorEje + 1);

    std::thread controlador([&] {
        while (atendidos.load() < totalVehiculos) {
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

    auto despachar = [&](ColaConcurrente<Vehiculo>& carril, Eje eje,
                         std::atomic<int>& atendidosEje) {
        while (auto siguiente = carril.extraer()) {
            interseccion.esperarPaso(eje);
            mostrar("[PASO] " + std::string(nombreEje(eje)) +
                    " | Vehiculo " + std::to_string(siguiente->id) + " cruza");
            std::this_thread::sleep_for(75ms);
            ++atendidosEje;
            ++atendidos;
        }
    };

    std::thread despachadorNS(despachar, std::ref(carrilNS), Eje::NorteSur,
                             std::ref(atendidosNS));
    std::thread despachadorEO(despachar, std::ref(carrilEO), Eje::EsteOeste,
                             std::ref(atendidosEO));

    generadorNS.join();
    generadorEO.join();
    despachadorNS.join();
    despachadorEO.join();
    controlador.join();

    mostrar("[FIN] Total: " + std::to_string(atendidos.load()) +
            "/" + std::to_string(totalVehiculos) +
            " | Norte-Sur: " + std::to_string(atendidosNS.load()) +
            " | Este-Oeste: " + std::to_string(atendidosEO.load()));
    return 0;
}
