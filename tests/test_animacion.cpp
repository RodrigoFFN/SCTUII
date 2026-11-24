#include "TrafficAnimationModel.hpp"

#include <cassert>

int main() {
    TrafficAnimationModel a;
    a.registrar("llegada", true, 1, 30);
    a.registrar("llegada", false, 7, 40);
    assert(a.enCola(true) == 1);
    assert(a.enCola(false) == 1);
    assert(a.visibles(60).size() == 2);

    a.registrar("cruce_inicio", true, 1, 130);
    assert(a.enCola(true) == 0);
    assert(a.visibles(180).size() == 2);

    // El cruce del vehiculo 1 termina y permanece brevemente en la animacion.
    a.registrar("paso", true, 1, 260);
    assert(a.visibles(550).size() == 2);
    assert(a.visibles(1050).size() == 1);

    // Los eventos se procesan una sola vez y la cola no se duplica.
    a.registrar("llegada", false, 7, 40);
    assert(a.enCola(false) == 1);
    assert(a.totalRegistrados() == 2);

    a.clear();
    assert(a.visibles(2000).empty());
    assert(a.totalRegistrados() == 0);
}
