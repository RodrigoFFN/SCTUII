#include "PoliticaSemaforo.hpp"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>

int main() {
    ConfiguracionSimulacion config;
    const int base = config.duracionVerdeMs;
    assert(PoliticaSemaforo::duracionVerde(config, 10, 0) == base);
    config.adaptativo = true;
    assert(PoliticaSemaforo::duracionVerde(config, 10, 0) > base);
    assert(PoliticaSemaforo::duracionVerde(config, 0, 10) < base);
    assert(PoliticaSemaforo::duracionVerde(config, 100, 0) <= base * 2);
    assert(PoliticaSemaforo::duracionVerde(config, 0, 100) >= base / 2);
}
