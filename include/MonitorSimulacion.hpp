#pragma once

#include "ConfiguracionSimulacion.hpp"
#include "Eventos.hpp"

#include <chrono>
#include <deque>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <string>

// Estado protegido por mutex para publicar instantaneas JSON al navegador.
class MonitorSimulacion {
public:
    void iniciar(const ConfiguracionSimulacion& configuracion) {
        std::lock_guard<std::mutex> bloqueo(mutex_);
        config_ = configuracion;
        resumen_ = {};
        estado_ = {Luz::Rojo, Luz::Rojo, false};
        eventos_.clear();
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
        eventos_.push_front(evento);
        if (eventos_.size() > 80) eventos_.pop_back();
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
        eventos_.clear();
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
        out << "{\"running\":" << (ejecutando_ ? "true" : "false")
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
            << ",\"events\":[";
        bool primero = true;
        for (const auto& evento : eventos_) {
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
    std::deque<EventoSimulacion> eventos_;
    std::string error_;
    bool ejecutando_ = false;
    bool terminado_ = false;
    bool detenido_ = false;
    std::chrono::steady_clock::time_point inicio_ = std::chrono::steady_clock::now();
    long long duracionFinalMs_ = 0;
};
