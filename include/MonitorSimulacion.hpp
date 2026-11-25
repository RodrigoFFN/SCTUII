#pragma once

#include "ConfiguracionSimulacion.hpp"
#include "Eventos.hpp"

#include <chrono>
#include <iomanip>
#include <mutex>
#include <map>
#include <sstream>
#include <string>
#include <vector>

// Instantaneas coherentes para las interfaces, protegidas por el mismo mutex.
class MonitorSimulacion {
public:
    struct EstadoVehiculo {
        Eje eje = Eje::NorteSur;
        int etapa = 0;
        int ms = 0;
        int llegada = -1, cruce = -1, salida = -1;
        std::string tipo;
    };
    struct Instantanea {
        ConfiguracionSimulacion config{};
        ResumenEstadisticas resumen{};
        EstadoInterseccion estado{Luz::Rojo, Luz::Rojo, false};
        std::map<int, EstadoVehiculo> vehiculos;
        std::vector<EventoSimulacion> historial;
        unsigned long long ejecucion = 0;
        bool ejecutando = false, terminado = false, detenido = false;
        long long tiempoMs = 0;
        std::string error;
    };
    Instantanea instantanea() const {
        std::lock_guard<std::mutex> bloqueo(mutex_);
        return {config_, resumen_, estado_, vehiculos_, historial_, ejecucion_,
                ejecutando_, terminado_, detenido_,
                ejecutando_ ? elapsedUnsafe() : duracionFinalMs_, error_};
    }

    void iniciar(const ConfiguracionSimulacion& configuracion) {
        std::lock_guard<std::mutex> bloqueo(mutex_);
        config_ = configuracion;
        resumen_ = {};
        estado_ = {Luz::Rojo, Luz::Rojo, false};
        historial_.clear();
        vehiculos_.clear();
        ++ejecucion_;
        error_.clear();
        ejecutando_ = true;
        terminado_ = false;
        detenido_ = false;
        inicio_ = std::chrono::steady_clock::now();
        duracionFinalMs_ = 0;
    }

    void registrar(const EventoSimulacion& evento,
                   const ResumenEstadisticas& resumen,
                   const EstadoInterseccion& estado) {
        std::lock_guard<std::mutex> bloqueo(mutex_);
        resumen_ = resumen;
        estado_ = estado;
        // Historial completo de estados individuales, independiente de la bitacora limitada.
        const int etapa = evento.tipo == "llegada" ? 1 : evento.tipo == "cruce" ? 2 :
                          evento.tipo == "paso" ? 3 : 0;
        if (etapa && evento.vehiculoId > 0) {
            auto& v = vehiculos_[evento.vehiculoId];
            v.eje = evento.eje;
            if (etapa >= v.etapa) {
                v.etapa = etapa;
                v.ms = evento.transcurridosMs;
            }
            if (etapa == 1) {
                v.llegada = evento.transcurridosMs;
                v.tipo = evento.tipoVehiculo;
            }
            if (etapa == 2) v.cruce = evento.transcurridosMs;
            if (etapa == 3) v.salida = evento.transcurridosMs;
        }
        historial_.push_back(evento);
    }

    void finalizar(const ResumenEstadisticas& resumen, bool detenido) {
        std::lock_guard<std::mutex> bloqueo(mutex_);
        resumen_ = resumen;
        ejecutando_ = false;
        terminado_ = true;
        detenido_ = detenido;
        estado_ = {Luz::Rojo, Luz::Rojo, false};
        duracionFinalMs_ = elapsedUnsafe();
    }

    void fallar(const std::string& mensaje) {
        std::lock_guard<std::mutex> bloqueo(mutex_);
        error_ = mensaje;
        ejecutando_ = false;
        terminado_ = true;
        duracionFinalMs_ = elapsedUnsafe();
    }

    bool ejecutando() const {
        std::lock_guard<std::mutex> bloqueo(mutex_);
        return ejecutando_;
    }

    void reiniciarVista() {
        std::lock_guard<std::mutex> bloqueo(mutex_);
        if (ejecutando_) return;
        resumen_ = {};
        estado_ = {Luz::Rojo, Luz::Rojo, false};
        historial_.clear();
        vehiculos_.clear();
        ++ejecucion_;
        error_.clear();
        terminado_ = false;
        detenido_ = false;
        duracionFinalMs_ = 0;
    }

    static std::string escapar(const std::string& texto) {
        std::ostringstream salida;
        for (unsigned char c : texto) {
            if (c == '"') salida << "\\\"";
            else if (c == '\\') salida << "\\\\";
            else if (c == '\n') salida << "\\n";
            else if (c == '\r') salida << "\\r";
            else if (c == '\t') salida << "\\t";
            else if (c < 0x20) {
                salida << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                       << int(c) << std::dec;
            } else salida << c;
        }
        return salida.str();
    }

    std::string json() const {
        std::lock_guard<std::mutex> bloqueo(mutex_);
        const std::string fase = estado_.norteSur == Luz::Verde ? "ns" :
                                  estado_.esteOeste == Luz::Verde ? "eo" : "rojo";
        std::ostringstream out;
        out << std::fixed << std::setprecision(1);
        out << "{\"run_id\":" << ejecucion_ << ",\"running\":" << (ejecutando_ ? "true" : "false")
            << ",\"finished\":" << (terminado_ ? "true" : "false")
            << ",\"stopped\":" << (detenido_ ? "true" : "false")
            << ",\"phase\":\"" << fase << "\""
            << ",\"occupied\":" << (estado_.ocupado ? "true" : "false")
            << ",\"elapsed_ms\":" << (ejecutando_ ? elapsedUnsafe() : duracionFinalMs_)
            << ",\"error\":\"" << escapar(error_) << "\""
            << ",\"generated\":" << resumen_.totalGenerados()
            << ",\"served\":" << resumen_.totalAtendidos()
            << ",\"pending\":" << resumen_.pendientes()
            << ",\"avg_wait_ms\":" << resumen_.esperaPromedio()
            << ",\"ns\":{\"generated\":" << resumen_.generadosNS
            << ",\"served\":" << resumen_.atendidosNS
            << ",\"wait_ms\":" << resumen_.esperaPromedioNS() << "}"
            << ",\"eo\":{\"generated\":" << resumen_.generadosEO
            << ",\"served\":" << resumen_.atendidosEO
            << ",\"wait_ms\":" << resumen_.esperaPromedioEO() << "}"
            << ",\"config\":{\"vehicles\":" << config_.vehiculosPorEje
            << ",\"arrival\":" << config_.intervaloLlegadaMs
            << ",\"crossing\":" << config_.tiempoCruceMs
            << ",\"green\":" << config_.duracionVerdeMs
            << ",\"red\":" << config_.duracionTodoRojoMs
            << ",\"adaptive\":" << (config_.adaptativo ? "true" : "false") << "}"
            << ",\"vehicles\":[";
        bool primerVehiculo = true;
        for (const auto& par : vehiculos_) {
            if (!primerVehiculo) out << ',';
            primerVehiculo = false;
            const auto& v = par.second;
            out << "{\"id\":" << par.first
                << ",\"axis\":\"" << (v.eje == Eje::NorteSur ? "ns" : "eo")
                << "\",\"stage\":\"" << (v.etapa == 1 ? "waiting" : v.etapa == 2 ? "crossing" : "served")
                << "\",\"ms\":" << v.ms
                << ",\"vehicle_type\":\"" << escapar(v.tipo) << "\""
                << ",\"arrival_ms\":" << v.llegada
                << ",\"cross_ms\":" << v.cruce
                << ",\"served_ms\":" << v.salida << '}';
        }
        out << "],\"events\":[";
        bool primero = true;
        int cantidadEventos = 0;
        for (auto it = historial_.rbegin(); it != historial_.rend() && cantidadEventos < 80; ++it, ++cantidadEventos) {
            const auto& evento = *it;
            if (!primero) out << ',';
            primero = false;
            out << "{\"type\":\"" << escapar(evento.tipo) << "\""
                << ",\"axis\":\"" << (evento.eje == Eje::NorteSur ? "ns" : "eo") << "\""
                << ",\"id\":" << evento.vehiculoId
                << ",\"text\":\"" << escapar(evento.descripcion) << "\""
                << ",\"ms\":" << evento.transcurridosMs << '}';
        }
        out << "]}";
        return out.str();
    }

private:
    long long elapsedUnsafe() const {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - inicio_).count();
    }
    mutable std::mutex mutex_;
    ConfiguracionSimulacion config_{};
    ResumenEstadisticas resumen_{};
    EstadoInterseccion estado_{Luz::Rojo, Luz::Rojo, false};
    std::map<int, EstadoVehiculo> vehiculos_;
    unsigned long long ejecucion_ = 0;
    std::vector<EventoSimulacion> historial_;
    std::string error_;
    bool ejecutando_ = false;
    bool terminado_ = false;
    bool detenido_ = false;
    std::chrono::steady_clock::time_point inicio_ = std::chrono::steady_clock::now();
    long long duracionFinalMs_ = 0;
};
