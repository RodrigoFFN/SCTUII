#include "ConfiguracionSimulacion.hpp"
#include <cassert>
#include <stdexcept>

int main() {
    ConfiguracionSimulacion normal;
    normal.validar();
    assert(normal.vehiculosPorEje == 6);
    assert(!normal.adaptativo);
    auto invalido = normal;
    invalido.vehiculosPorEje = 0;
    bool rechazado = false;
    try { invalido.validar(); } catch (const std::invalid_argument&) { rechazado = true; }
    assert(rechazado);
    invalido = normal;
    invalido.duracionVerdeMs = 50;
    rechazado = false;
    try { invalido.validar(); } catch (const std::invalid_argument&) { rechazado = true; }
    assert(rechazado);
    invalido.duracionVerdeMs = 500;
    invalido.adaptativo = true;
    invalido.validar();
}
