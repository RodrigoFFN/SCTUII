#pragma once

#include "ConfiguracionSimulacion.hpp"
#include <algorithm>

class PoliticaSemaforo {
public:
    // Estrategia heuristica; no predice congestiones ni optimiza una red vial real.
    static int duracionVerde(const ConfiguracionSimulacion& config,
                             int pendientesEsteEje, int pendientesOtroEje) {
        if (!config.adaptativo) return config.duracionVerdeMs;
        const int diferencial = pendientesEsteEje - pendientesOtroEje;
        const int objetivo = config.duracionVerdeMs + diferencial * 110;
        const int minimo = std::max(100, config.duracionVerdeMs / 2);
        const int maximo = std::min(10000, config.duracionVerdeMs * 2);
        return std::max(minimo, std::min(maximo, objetivo));
    }
};
