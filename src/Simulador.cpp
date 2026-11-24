#include "Simulador.hpp"

#include "Carril.hpp"
#include "FabricaVehiculos.hpp"
#include "PoliticaSemaforo.hpp"

#include <algorithm>
#include <chrono>
#include <exception>
#include <future>
#include <iostream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

using RelojSimulacion = std::chrono::steady_clock;

namespace {
const char* nombreEje(Eje eje) {
    return eje == Eje::NorteSur ? "Norte-Sur" : "Este-Oeste";
}

void imprimirResumen(const ResumenEstadisticas& resumen) {
    std::cout << "\n========== RESUMEN SIMTRRAFIC ==========\n"
              << "Generados: " << resumen.totalGenerados() << '\n'
              << "Atendidos: " << resumen.totalAtendidos() << '\n'
              << "Norte-Sur: " << resumen.atendidosNS << "/" << resumen.generadosNS << '\n'
              << "Este-Oeste: " << resumen.atendidosEO << "/" << resumen.generadosEO << '\n'
              << "Pendientes: " << resumen.pendientes() << '\n'
              << "Espera media estimada: " << resumen.esperaPromedio() << " ms\n"
              << "=======================================\n";
}
}

Simulador::Simulador(ConfiguracionSimulacion config,
                     ObservadorSimulacion observador)
    : config_(config), observador_(std::move(observador)) {
    config_.validar();
}

bool Simulador::esperarOParar(std::chrono::milliseconds duracion) const {
    std::unique_lock<std::mutex> bloqueo(mutexParada_);
    return avisoParada_.wait_for(bloqueo, duracion, [this] { return parar_.load(); });
}

void Simulador::solicitarDetencion() const {
    parar_.store(true);
    avisoParada_.notify_all();
    std::shared_ptr<Interseccion> activo;
    {
        std::lock_guard<std::mutex> bloqueo(mutexInterseccion_);
        activo = interseccionActual_;
    }
    if (activo) activo->cancelar();
}

ResumenEstadisticas Simulador::ejecutar() const {
    Carril carrilNS(Eje::NorteSur);
    Carril carrilEO(Eje::EsteOeste);
    auto interseccion = std::make_shared<Interseccion>();
    Estadisticas estadisticas;
    std::mutex mutexSalida;
    const auto inicio = RelojSimulacion::now();
    const int cantidad = config_.vehiculosPorEje;
    const int total = cantidad * 2;

    {
        std::lock_guard<std::mutex> bloqueo(mutexInterseccion_);
        interseccionActual_ = interseccion;
    }

    auto publicar = [&](const std::string& tipo, Eje eje, int id,
                        const std::string& mensaje) {
        if (config_.mostrarConsola) {
            std::lock_guard<std::mutex> bloqueo(mutexSalida);
            std::cout << mensaje << '\n';
        }
        if (observador_) {
            const int transcurridos = static_cast<int>(
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    RelojSimulacion::now() - inicio).count());
            observador_({tipo, eje, id, mensaje, transcurridos},
                        estadisticas.resumenFinal(), interseccion->estado());
        }
    };

    publicar("inicio", Eje::NorteSur, 0, "Simulacion iniciada");

    auto generar = [&](Carril& carril, int idInicial) {
        const Eje eje = carril.eje();
        for (int i = 0; i < cantidad && !parar_.load(); ++i) {
            const auto vehiculo = FabricaVehiculos::crear(idInicial + i);
            if (carril.agregar(vehiculo)) {
                estadisticas.registrarGenerado(eje);
                publicar("llegada", eje, vehiculo.id,
                         "[LLEGADA] " + std::string(nombreEje(eje)) +
                         " | " + vehiculo.tipo + " #" + std::to_string(vehiculo.id));
            }
            if (esperarOParar(std::chrono::milliseconds(config_.intervaloLlegadaMs))) break;
        }
        carril.cerrar();
    };

    auto despachar = [&](Carril& carril) {
        const Eje eje = carril.eje();
        while (!parar_.load()) {
            auto vehiculo = carril.extraer();
            if (!vehiculo) break;
            // El evento se emite exactamente despues de reservar el cruce.
            // Qt anima el vehiculo a partir de este instante real del motor.
            if (!interseccion->cruzar(
                    eje, std::chrono::milliseconds(config_.tiempoCruceMs), [&] {
                        publicar("cruce_inicio", eje, vehiculo->id,
                                 "[CRUCE] " + std::string(nombreEje(eje)) +
                                 " | Vehiculo #" + std::to_string(vehiculo->id));
                    })) break;
            const auto transcurridos = std::chrono::duration_cast<std::chrono::milliseconds>(
                RelojSimulacion::now() - vehiculo->llegada).count();
            // El tiempo de cola se estima restando el tiempo de ocupacion.
            estadisticas.registrarEspera(eje,
                std::max<std::int64_t>(0, transcurridos - config_.tiempoCruceMs));
            estadisticas.registrarAtendido(eje);
            publicar("paso", eje, vehiculo->id,
                     "[PASO] " + std::string(nombreEje(eje)) +
                     " | Vehiculo #" + std::to_string(vehiculo->id));
        }
    };

    auto fase = [&](Eje eje) {
        interseccion->detener();
        publicar("fase", eje, 0, "[SEMAFOROS] Transicion: ambos ROJO");
        if (esperarOParar(std::chrono::milliseconds(config_.duracionTodoRojoMs))) return;

        const auto resumen = estadisticas.resumenFinal();
        const int propios = eje == Eje::NorteSur
            ? resumen.generadosNS - resumen.atendidosNS
            : resumen.generadosEO - resumen.atendidosEO;
        const int ajenos = eje == Eje::NorteSur
            ? resumen.generadosEO - resumen.atendidosEO
            : resumen.generadosNS - resumen.atendidosNS;
        const int duracion = PoliticaSemaforo::duracionVerde(config_, propios, ajenos);
        if (parar_.load()) return;
        interseccion->habilitar(eje);
        if (parar_.load()) return;
        publicar("fase", eje, 0,
                 "[SEMAFOROS] " + std::string(nombreEje(eje)) + " VERDE (" +
                 std::to_string(duracion) + " ms)");
        esperarOParar(std::chrono::milliseconds(duracion));
    };

    auto controlarSemaforos = [&] {
        while (!parar_.load() && estadisticas.totalAtendidos() < total) {
            fase(Eje::NorteSur);
            if (parar_.load() || estadisticas.totalAtendidos() >= total) break;
            fase(Eje::EsteOeste);
        }
        interseccion->detener();
        publicar("fase", Eje::NorteSur, 0, "[SEMAFOROS] Final: ambos ROJO");
    };

    std::promise<bool> permiso;
    const auto listo = permiso.get_future().share();
    std::vector<std::thread> hilos;
    hilos.reserve(5);
    std::exception_ptr error;
    std::mutex mutexError;

    auto lanzar = [&](auto funcion) {
        hilos.emplace_back([&, listo, funcion = std::move(funcion)] {
            if (!listo.get()) return;
            try { funcion(); }
            catch (...) {
                {
                    std::lock_guard<std::mutex> bloqueo(mutexError);
                    if (!error) error = std::current_exception();
                }
                solicitarDetencion();
                carrilNS.cerrar();
                carrilEO.cerrar();
            }
        });
    };

    std::exception_ptr falloCreacion;
    try {
        lanzar([&] { generar(carrilNS, 1); });
        lanzar([&] { generar(carrilEO, cantidad + 1); });
        lanzar([&] { despachar(carrilNS); });
        lanzar([&] { despachar(carrilEO); });
        lanzar(controlarSemaforos);
    } catch (...) {
        falloCreacion = std::current_exception();
    }
    permiso.set_value(!falloCreacion);
    for (auto& hilo : hilos) hilo.join();

    {
        std::lock_guard<std::mutex> bloqueo(mutexInterseccion_);
        interseccionActual_.reset();
    }

    if (falloCreacion) std::rethrow_exception(falloCreacion);
    if (error) std::rethrow_exception(error);

    const auto resumen = estadisticas.resumenFinal();
    publicar(parar_.load() ? "detenido" : "fin", Eje::NorteSur, 0,
             parar_.load() ? "Simulacion detenida" : "Simulacion completada");
    if (config_.mostrarConsola) {
        std::lock_guard<std::mutex> bloqueo(mutexSalida);
        imprimirResumen(resumen);
    }
    return resumen;
}
