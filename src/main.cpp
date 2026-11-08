#include "ColaConcurrente.hpp"
#include "Semaforo.hpp"
#include "Vehiculo.hpp"

#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

using namespace std::chrono_literals;

int main() {
    constexpr int totalVehiculos = 8;
    ColaConcurrente<Vehiculo> carril;
    Semaforo semaforo;
    std::atomic<int> atendidos{0};
    std::mutex mutexSalida;

    auto mostrar = [&](const std::string& mensaje) {
        std::lock_guard<std::mutex> bloqueo(mutexSalida);
        std::cout << mensaje << '\n';
    };

    mostrar("SIM-TRAFIC | Avance 01 | Simulacion de un carril");

    std::thread generador([&] {
        for (int i = 1; i <= totalVehiculos; ++i) {
            Vehiculo vehiculo{i, (i % 3 == 0) ? "Bus" : "Auto"};
            if (carril.agregar(vehiculo)) {
                mostrar("[LLEGADA] Vehiculo " + std::to_string(i) +
                        " (" + vehiculo.tipo + ") entra en la cola");
            }
            std::this_thread::sleep_for(60ms);
        }
        carril.cerrar();
    });

    std::thread controlador([&] {
        while (atendidos.load() < totalVehiculos) {
            semaforo.cambiarA(Luz::Rojo);
            mostrar("[SEMAFORO] ROJO: acceso detenido");
            std::this_thread::sleep_for(180ms);

            semaforo.cambiarA(Luz::Verde);
            mostrar("[SEMAFORO] VERDE: acceso permitido");
            std::this_thread::sleep_for(300ms);
        }
    });

    std::thread despachador([&] {
        while (auto siguiente = carril.extraer()) {
            semaforo.esperarVerde();
            mostrar("[PASO] Vehiculo " + std::to_string(siguiente->id) +
                    " cruza la interseccion");
            std::this_thread::sleep_for(75ms);
            ++atendidos;
        }
    });

    generador.join();
    despachador.join();
    controlador.join();

    mostrar("[FIN] Vehiculos atendidos: " + std::to_string(atendidos.load()));
    return 0;
}

