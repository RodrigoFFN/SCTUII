#include "MonitorSimulacion.hpp"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <string>

int main() {
    MonitorSimulacion m;
    ConfiguracionSimulacion c;
    m.iniciar(c);
    assert(m.ejecutando());
    m.registrar({"llegada", Eje::NorteSur, 4, "Auto \"rapido\"", 30},
                 {1, 0, 0, 0}, {Luz::Verde, Luz::Rojo, false});
    const std::string json = m.json();
    assert(json.find("\"phase\":\"ns\"") != std::string::npos);
    assert(json.find("\\\"rapido\\\"") != std::string::npos);
    assert(json.find("\"pending\":1") != std::string::npos);
    m.finalizar({1, 0, 1, 0}, false);
    assert(!m.ejecutando());
    assert(m.json().find("\"finished\":true") != std::string::npos);
    m.reiniciarVista();
    assert(m.json().find("\"finished\":false") != std::string::npos);
}
