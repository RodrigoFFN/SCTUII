#pragma once

#include "MonitorSimulacion.hpp"
#include "Simulador.hpp"

#include <memory>
#include <string>
#include <thread>

// Servidor HTTP local: el navegador es la vista; el motor sigue siendo C++17.
class ServidorWeb {
public:
    explicit ServidorWeb(int puerto = 8080);
    ~ServidorWeb();
    int ejecutar();

private:
    void iniciarSimulacion(const ConfiguracionSimulacion& config);
    int puerto_;
    MonitorSimulacion monitor_;
    std::shared_ptr<Simulador> simulador_;
    std::thread hilo_;
};
