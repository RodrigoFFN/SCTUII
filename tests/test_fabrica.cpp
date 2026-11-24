#include "FabricaVehiculos.hpp"
#include <cassert>

int main() {
    assert(FabricaVehiculos::crear(1).tipo == "Auto");
    assert(FabricaVehiculos::crear(3).tipo == "Bus");
    assert(FabricaVehiculos::crear(6).id == 6);
}
