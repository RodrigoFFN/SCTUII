#include "Simulador.hpp"
#include "ServidorWeb.hpp"

#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
void ayuda(const char* ejecutable) {
    std::cout << "SIMTRRAFIC - Simulador concurrente de trafico\n\n"
              << "Uso: " << ejecutable << " [opciones]\n"
              << "  --vehiculos N    Vehiculos por eje (1-60; predeterminado 6)\n"
              << "  --llegada-ms N   Separacion entre llegadas (10-5000)\n"
              << "  --cruce-ms N     Tiempo por vehiculo (20-5000)\n"
              << "  --verde-ms N     Tiempo base de verde (100-10000)\n"
              << "  --rojo-ms N      Tiempo de seguridad todo rojo (20-5000)\n"
              << "  --adaptativo     Ajuste heuristico de verde segun la demanda\n"
              << "  --fijo           Ciclo fijo (predeterminado)\n"
              << "  --gui            Abrir servidor de interfaz local en navegador\n"
              << "  --port N         Puerto del panel (1024-65535; predeterminado 8080)\n"
              << "  --help           Ver esta ayuda\n";
}
int enteroPositivo(const std::string& texto) {
    std::size_t usado = 0;
    int valor = std::stoi(texto, &usado);
    if (usado != texto.size()) throw std::invalid_argument("Numero invalido: " + texto);
    return valor;
}
}

int main(int argc, char* argv[]) {
    try {
        ConfiguracionSimulacion config;
        bool interfaz = false;
        int puerto = 8080;
        for (int i = 1; i < argc; ++i) {
            const std::string opcion = argv[i];
            if (opcion == "--help" || opcion == "-h") {
                ayuda(argv[0]);
                return 0;
            }
            if (opcion == "--gui") { interfaz = true; continue; }
            if (opcion == "--port") {
                if (i + 1 == argc) throw std::invalid_argument("Falta valor del puerto");
                puerto = enteroPositivo(argv[++i]);
                continue;
            }
            if (opcion == "--adaptativo") { config.adaptativo = true; continue; }
            if (opcion == "--fijo") { config.adaptativo = false; continue; }
            if (opcion != "--vehiculos" && opcion != "--llegada-ms" &&
                opcion != "--cruce-ms" && opcion != "--verde-ms" &&
                opcion != "--rojo-ms") {
                throw std::invalid_argument("Opcion desconocida: " + opcion);
            }
            if (i + 1 == argc) throw std::invalid_argument("Falta valor de " + opcion);
            const int valor = enteroPositivo(argv[++i]);
            if (opcion == "--vehiculos") config.vehiculosPorEje = valor;
            if (opcion == "--llegada-ms") config.intervaloLlegadaMs = valor;
            if (opcion == "--cruce-ms") config.tiempoCruceMs = valor;
            if (opcion == "--verde-ms") config.duracionVerdeMs = valor;
            if (opcion == "--rojo-ms") config.duracionTodoRojoMs = valor;
        }
        config.validar();
        if (interfaz) return ServidorWeb(puerto).ejecutar();
        const auto resumen = Simulador(config).ejecutar();
        return resumen.totalAtendidos() == 2 * config.vehiculosPorEje ? 0 : 1;
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << "\nUse --help para ver los parametros.\n";
        return 1;
    }
}
