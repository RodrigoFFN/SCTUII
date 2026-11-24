#pragma once

#include "Vehiculo.hpp"

// Fabrica centralizada: permite variar tipos sin cambiar los generadores.
class FabricaVehiculos {
public:
    static Vehiculo crear(int identificador) {
        return {identificador, identificador % 3 == 0 ? "Bus" : "Auto"};
    }
};
