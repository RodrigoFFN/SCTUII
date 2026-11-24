#include "Simulador.hpp"

#include <cassert>
#include <mutex>
#include <set>
#include <utility>

int main() {
    ConfiguracionSimulacion config;
    config.vehiculosPorEje = 3;
    config.intervaloLlegadaMs = 10;
    config.tiempoCruceMs = 25;
    config.duracionVerdeMs = 100;
    config.duracionTodoRojoMs = 20;
    config.mostrarConsola = false;

    std::mutex mutex;
    std::set<int> entradas;
    std::set<int> salidas;
    bool todasConReserva = true;
    bool enFaseCorrecta = true;

    Simulador simulador(config, [&](const EventoSimulacion& evento,
                                    const ResumenEstadisticas&,
                                    const EstadoInterseccion& estado) {
        std::lock_guard<std::mutex> lock(mutex);
        if (evento.tipo == "cruce_inicio") {
            entradas.insert(evento.vehiculoId);
            if (!estado.ocupado) todasConReserva = false;
            if (evento.eje == Eje::NorteSur) {
                if (estado.norteSur != Luz::Verde || estado.esteOeste != Luz::Rojo)
                    enFaseCorrecta = false;
            } else {
                if (estado.esteOeste != Luz::Verde || estado.norteSur != Luz::Rojo)
                    enFaseCorrecta = false;
            }
        }
        if (evento.tipo == "paso") salidas.insert(evento.vehiculoId);
    });

    const auto resumen = simulador.ejecutar();
    assert(resumen.totalGenerados() == 6);
    assert(resumen.totalAtendidos() == 6);
    assert(resumen.pendientes() == 0);
    assert(entradas.size() == 6);
    assert(salidas.size() == 6);
    assert(entradas == salidas);
    assert(todasConReserva);
    assert(enFaseCorrecta);
}
