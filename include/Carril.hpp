#pragma once

#include "ColaConcurrente.hpp"
#include "Interseccion.hpp"
#include "Vehiculo.hpp"

#include <optional>
#include <utility>

class Carril {
public:
    explicit Carril(Eje eje) : eje_(eje) {}

    Eje eje() const noexcept { return eje_; }

    bool agregar(Vehiculo vehiculo) {
        return vehiculos_.agregar(std::move(vehiculo));
    }

    // Espera un vehículo o devuelve nullopt cuando la cola está cerrada y vacía.
    std::optional<Vehiculo> extraer() {
        return vehiculos_.extraer();
    }

    void cerrar() {
        vehiculos_.cerrar();
    }

private:
    Eje eje_;
    ColaConcurrente<Vehiculo> vehiculos_;
};
