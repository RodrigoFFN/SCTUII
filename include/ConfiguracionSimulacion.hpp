#pragma once

#include <stdexcept>

// Unidades en milisegundos. Los limites evitan una interfaz bloqueada por entradas extremas.
struct ConfiguracionSimulacion {
    int vehiculosPorEje = 6;
    int intervaloLlegadaMs = 90;
    int tiempoCruceMs = 130;
    int duracionVerdeMs = 1000;
    int duracionTodoRojoMs = 220;
    bool adaptativo = false;
    bool mostrarConsola = true;

    void validar() const {
        if (vehiculosPorEje < 1 || vehiculosPorEje > 60)
            throw std::invalid_argument("Vehiculos por eje: rango valido 1-60");
        if (intervaloLlegadaMs < 10 || intervaloLlegadaMs > 5000)
            throw std::invalid_argument("Intervalo de llegada: rango valido 10-5000 ms");
        if (tiempoCruceMs < 20 || tiempoCruceMs > 5000)
            throw std::invalid_argument("Tiempo de cruce: rango valido 20-5000 ms");
        if (duracionVerdeMs < 100 || duracionVerdeMs > 10000)
            throw std::invalid_argument("Duracion verde: rango valido 100-10000 ms");
        if (duracionTodoRojoMs < 20 || duracionTodoRojoMs > 5000)
            throw std::invalid_argument("Todo rojo: rango valido 20-5000 ms");
    }
};
