#pragma once

#include <string>
#include <chrono>

struct Vehiculo {
    int id;
    std::string tipo;
    std::chrono::steady_clock::time_point llegada = std::chrono::steady_clock::now();
};

