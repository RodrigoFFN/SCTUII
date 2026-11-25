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
    m.registrar({"llegada", Eje::NorteSur, 4, "Auto \"rapido\"", 30, "Auto"},
                 {1, 0, 0, 0}, {Luz::Verde, Luz::Rojo, false});
    const std::string json = m.json();
    auto snapshot = m.instantanea();
    assert(snapshot.ejecucion == 1 && snapshot.ejecutando);
    assert(snapshot.vehiculos.at(4).tipo == "Auto");
    assert(snapshot.vehiculos.at(4).llegada == 30);
    assert(json.find("\"vehicle_type\":\"Auto\"") != std::string::npos);
    assert(json.find("\"phase\":\"ns\"") != std::string::npos);
    assert(json.find("\\\"rapido\\\"") != std::string::npos);
    assert(json.find("\"pending\":1") != std::string::npos);
    m.registrar({"cruce", Eje::NorteSur, 4, "En cruce", 40},
                 {1, 0, 0, 0}, {Luz::Verde, Luz::Rojo, true});
    assert(m.json().find("\"stage\":\"crossing\"") != std::string::npos);
    m.registrar({"paso", Eje::NorteSur, 4, "Completado", 50},
                 {1, 0, 1, 0}, {Luz::Verde, Luz::Rojo, false});
    assert(m.json().find("\"stage\":\"served\"") != std::string::npos);
    for (int i=0; i<90; ++i)
        m.registrar({"fase", Eje::NorteSur, 0, "Fase", 60+i},
                    {1, 0, 1, 0}, {Luz::Rojo, Luz::Rojo, false});
    snapshot = m.instantanea();
    assert(snapshot.historial.size() == 93);
    assert(snapshot.vehiculos.at(4).cruce == 40 && snapshot.vehiculos.at(4).salida == 50);
    m.finalizar({1, 0, 1, 0}, false);
    assert(!m.ejecutando());
    assert(m.json().find("\"finished\":true") != std::string::npos);
    m.reiniciarVista();
    assert(m.json().find("\"finished\":false") != std::string::npos);
    assert(m.json().find("\"vehicles\":[]") != std::string::npos);
    assert(m.instantanea().historial.empty() && m.instantanea().ejecucion == 2);
}
