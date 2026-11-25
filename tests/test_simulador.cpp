#include "Simulador.hpp"
#include <atomic>
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <chrono>
#include <thread>

int main() {
    ConfiguracionSimulacion c;
    c.vehiculosPorEje = 2;
    c.intervaloLlegadaMs = 10;
    c.tiempoCruceMs = 20;
    c.duracionVerdeMs = 100;
    c.duracionTodoRojoMs = 20;
    c.adaptativo = true;
    c.mostrarConsola = false;
    std::atomic<int> llegadas{0}, pasos{0}, cruces{0}, fases{0};
    Simulador simulador(c, [&](const EventoSimulacion& evento, const ResumenEstadisticas& r,
                               const EstadoInterseccion& luces) {
        assert(!(luces.norteSur == Luz::Verde && luces.esteOeste == Luz::Verde));
        assert(r.pendientes() >= 0);
        if (evento.tipo == "llegada") {
            ++llegadas;
            assert(evento.tipoVehiculo == (evento.vehiculoId % 3 == 0 ? "Bus" : "Auto"));
        }
        if (evento.tipo == "paso") ++pasos;
        if (evento.tipo == "cruce") ++cruces;
        if (evento.tipo == "fase") ++fases;
    });
    const auto fin = simulador.ejecutar();
    assert(fin.totalGenerados() == 4);
    assert(fin.totalAtendidos() == 4);
    assert(fin.pendientes() == 0);
    assert(llegadas == 4 && pasos == 4 && cruces == 4 && fases > 0);
    assert(fin.esperaPromedio() >= 0.0);

    // Detencion de una simulacion larga: no debe quedarse bloqueada en un rojo.
    c.vehiculosPorEje = 60;
    c.duracionVerdeMs = 10000;
    c.intervaloLlegadaMs = 500;
    Simulador largo(c);
    std::thread hilo([&] {
        const auto resultado = largo.ejecutar();
        assert(resultado.totalGenerados() <= 120);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    largo.solicitarDetencion();
    hilo.join();
}
