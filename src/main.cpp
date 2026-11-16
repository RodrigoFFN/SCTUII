#include "Simulador.hpp"

#include <exception>
#include <iostream>

int main() {
    try {
        const Simulador simulador;
        simulador.ejecutar();
    } catch (const std::exception& error) {
        std::cerr << "Error en la simulacion: " << error.what() << '\n';
        return 1;
    }
    return 0;
}