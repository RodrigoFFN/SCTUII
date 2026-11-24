#include "Interseccion.hpp"
#include <atomic>
#include <cassert>
#include <chrono>
#include <thread>

int main() {
    using namespace std::chrono_literals;
    Interseccion cruce;
    cruce.habilitar(Eje::NorteSur);
    std::atomic<bool> inicio{false};
    std::thread vehiculo([&] {
        inicio.store(true);
        assert(cruce.cruzar(Eje::NorteSur, 90ms));
    });
    while (!inicio.load()) std::this_thread::yield();
    // Esperamos que el coche haya entrado realmente a la zona critica.
    for (int intento = 0; intento < 300 && !cruce.estado().ocupado; ++intento)
        std::this_thread::sleep_for(1ms);
    assert(cruce.estado().ocupado);
    auto ahora = std::chrono::steady_clock::now();
    cruce.habilitar(Eje::EsteOeste);
    auto demora = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - ahora).count();
    vehiculo.join();
    assert(demora >= 20);
    assert(!cruce.estado().ocupado);
    assert(cruce.estado().esteOeste == Luz::Verde);
    cruce.cancelar();
    assert(!cruce.cruzar(Eje::EsteOeste, 1ms));
}
