#include "ColaConcurrente.hpp"
#include "Semaforo.hpp"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

void verificar(bool condicion, const char* mensaje) {
    if (!condicion) {
        std::cerr << "FALLO: " << mensaje << '\n';
        std::abort();
    }
}

int main() {
    constexpr int productores = 4;
    constexpr int consumidores = 3;
    constexpr int porProductor = 100;
    constexpr int total = productores * porProductor;

    ColaConcurrente<int> cola;
    std::vector<int> apariciones(total, 0);
    std::mutex mutexResultados;
    std::vector<std::thread> hilos;

    for (int i = 0; i < consumidores; ++i) {
        hilos.emplace_back([&] {
            while (auto elemento = cola.extraer()) {
                std::lock_guard<std::mutex> bloqueo(mutexResultados);
                verificar(*elemento >= 0 && *elemento < total, "indice fuera de rango");
                ++apariciones[*elemento];
            }
        });
    }

    for (int i = 0; i < productores; ++i) {
        hilos.emplace_back([&, i] {
            for (int j = 0; j < porProductor; ++j) {
                const bool aceptado = cola.agregar(i * porProductor + j);
                verificar(aceptado, "cola cerrada antes de terminar la produccion");
            }
        });
    }

    for (int i = consumidores; i < consumidores + productores; ++i) {
        hilos[i].join();
    }
    cola.cerrar();
    for (int i = 0; i < consumidores; ++i) {
        hilos[i].join();
    }

    for (int veces : apariciones) verificar(veces == 1, "duplicado o perdida");
    const bool aceptadoDespuesDeCerrar = cola.agregar(999);
    verificar(!aceptadoDespuesDeCerrar, "se acepto un elemento tras cerrar");
    verificar(!cola.extraer().has_value(), "cola cerrada no vacia");

    Semaforo semaforo;
    verificar(semaforo.actual() == Luz::Rojo, "el semaforo no inicia en rojo");
    std::atomic<bool> liberado{false};
    std::thread espera([&] {
        semaforo.esperarVerde();
        liberado.store(true);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    verificar(!liberado.load(), "el semaforo dejo pasar en rojo");
    semaforo.cambiarA(Luz::Verde);
    espera.join();
    verificar(liberado.load(), "el semaforo no libero el paso en verde");

    std::cout << "OK: 400 elementos sin duplicados ni perdidas; semaforo validado.\n";
}

