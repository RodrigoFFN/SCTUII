#include "Interseccion.hpp"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

void verificar(bool condicion, const char* mensaje) {
    if (!condicion) {
        std::cerr << "FALLO: " << mensaje << '\n';
        std::abort();
    }
}

void verificarExclusion(const EstadoInterseccion& estado) {
    verificar(!(estado.norteSur == Luz::Verde && estado.esteOeste == Luz::Verde),
             "dos ejes incompatibles recibieron verde");
}

int main() {
    using namespace std::chrono_literals;
    Interseccion interseccion;

    auto estado = interseccion.estado();
    verificar(estado.norteSur == Luz::Rojo && estado.esteOeste == Luz::Rojo,
             "la interseccion no inicia en rojo");

    interseccion.habilitar(Eje::NorteSur);
    estado = interseccion.estado();
    verificar(estado.norteSur == Luz::Verde && estado.esteOeste == Luz::Rojo,
             "fase norte-sur incorrecta");

    // Un vehiculo del otro eje no debe recibir permiso prematuramente.
    std::atomic<bool> inicio{false};
    std::atomic<bool> cruzo{false};
    std::thread vehiculo([&] {
        inicio.store(true);
        interseccion.esperarPaso(Eje::EsteOeste);
        cruzo.store(true);
    });
    while (!inicio.load()) std::this_thread::yield();
    std::this_thread::sleep_for(20ms);
    verificar(!cruzo.load(), "se autorizo el paso en rojo");

    interseccion.habilitar(Eje::EsteOeste);
    vehiculo.join();
    verificar(cruzo.load(), "el vehiculo no avanzo al cambiar a verde");
    estado = interseccion.estado();
    verificar(estado.norteSur == Luz::Rojo && estado.esteOeste == Luz::Verde,
             "fase este-oeste incorrecta");

    // Dos controladores compitiendo no deben dejar dos verdes a la vez.
    std::thread controladorNS([&] {
        for (int i = 0; i < 200; ++i) interseccion.habilitar(Eje::NorteSur);
    });
    std::thread controladorEO([&] {
        for (int i = 0; i < 200; ++i) interseccion.habilitar(Eje::EsteOeste);
    });
    for (int i = 0; i < 1000; ++i) verificarExclusion(interseccion.estado());
    controladorNS.join();
    controladorEO.join();
    verificarExclusion(interseccion.estado());

    interseccion.detener();
    estado = interseccion.estado();
    verificar(estado.norteSur == Luz::Rojo && estado.esteOeste == Luz::Rojo,
             "detener no apago ambas luces");

    std::cout << "OK: fases excluyentes y espera de vehiculos validadas.\n";
}
