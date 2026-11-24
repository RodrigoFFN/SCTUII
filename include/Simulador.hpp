#pragma once

#include "ConfiguracionSimulacion.hpp"
#include "Eventos.hpp"

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>

class Simulador {
public:
    explicit Simulador(ConfiguracionSimulacion config = {},
                       ObservadorSimulacion observador = {});

    // Ejecuta los generadores y despachadores, bloqueando hasta el final.
    ResumenEstadisticas ejecutar() const;

    // Puede llamarse desde el hilo de la interfaz. Termina sin dejar hilos sueltos.
    void solicitarDetencion() const;

private:
    bool esperarOParar(std::chrono::milliseconds duracion) const;
    ConfiguracionSimulacion config_;
    ObservadorSimulacion observador_;
    mutable std::atomic<bool> parar_{false};
    mutable std::mutex mutexParada_;
    mutable std::condition_variable avisoParada_;
    mutable std::mutex mutexInterseccion_;
    mutable std::shared_ptr<Interseccion> interseccionActual_;
};
