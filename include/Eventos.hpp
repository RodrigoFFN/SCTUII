#pragma once

#include "Estadisticas.hpp"
#include "Interseccion.hpp"

#include <functional>
#include <string>

struct EventoSimulacion {
    std::string tipo;      // inicio, llegada, fase, cruce, paso, fin, detenido
    Eje eje = Eje::NorteSur;
    int vehiculoId = 0;
    std::string descripcion;
    int transcurridosMs = 0;
    std::string tipoVehiculo; // Tipo real, informado al registrar la llegada.
};

// La vista escucha al motor sin que el motor dependa de HTTP o JavaScript.
using ObservadorSimulacion = std::function<void(
    const EventoSimulacion&, const ResumenEstadisticas&, const EstadoInterseccion&)>;
