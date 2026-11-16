#pragma once

#include "Estadisticas.hpp"

class Simulador {
public:
    // Ejecuta la simulación completa y devuelve el resumen cuando terminan los hilos.
    ResumenEstadisticas ejecutar() const;
};
