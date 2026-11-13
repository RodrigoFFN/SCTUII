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

    // Una cola cerrada debe permitir consumir los elementos que quedaron pendientes.
    ColaConcurrente<int> colaPendiente;
    verificar(colaPendiente.agregar(11), "no se pudo agregar el primer pendiente");
    verificar(colaPendiente.agregar(22), "no se pudo agregar el segundo pendiente");
    colaPendiente.cerrar();
    const auto primero = colaPendiente.extraer();
    const auto segundo = colaPendiente.extraer();
    verificar(primero.has_value() && *primero == 11, "se perdio el primer pendiente");
    verificar(segundo.has_value() && *segundo == 22, "se perdio el segundo pendiente");
    verificar(!colaPendiente.extraer().has_value(), "la cola cerrada no termino vacia");

    // Cerrar debe despertar al consumidor incluso si no llegaron elementos.
    ColaConcurrente<int> colaSinElementos;
    std::atomic<bool> consumidorIniciado{false};
    std::atomic<bool> consumidorFinalizado{false};
    std::thread consumidorBloqueado([&] {
        consumidorIniciado.store(true);
        const auto resultado = colaSinElementos.extraer();
        consumidorFinalizado.store(!resultado.has_value());
    });
    while (!consumidorIniciado.load()) std::this_thread::yield();
    colaSinElementos.cerrar();
    consumidorBloqueado.join();
    verificar(consumidorFinalizado.load(), "el cierre no libero al consumidor");

    std::cout << "OK: 400 elementos sin perdidas; cierre de cola y semaforo validados.\n";
}

