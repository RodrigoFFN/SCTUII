#include "ServidorWeb.hpp"

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
using Socket = SOCKET;
constexpr Socket SocketInvalido = INVALID_SOCKET;
void cerrarSocket(Socket s) { closesocket(s); }
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
using Socket = int;
constexpr Socket SocketInvalido = -1;
void cerrarSocket(Socket s) { close(s); }
#endif

#ifndef SIMTRAFFIC_WEB_ROOT
#define SIMTRAFFIC_WEB_ROOT "web"
#endif

namespace {
struct Peticion {
    std::string metodo;
    std::string ruta;
    std::string cuerpo;
};

std::string minusculas(std::string dato) {
    std::transform(dato.begin(), dato.end(), dato.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return dato;
}

std::string leerArchivo(const std::string& nombre) {
    std::ifstream entrada(std::string(SIMTRAFFIC_WEB_ROOT) + "/" + nombre,
                          std::ios::binary);
    if (!entrada) return {};
    return {std::istreambuf_iterator<char>(entrada), std::istreambuf_iterator<char>()};
}

std::string decodificar(const std::string& cadena) {
    std::string salida;
    for (std::size_t i = 0; i < cadena.size(); ++i) {
        if (cadena[i] == '+') salida.push_back(' ');
        else if (cadena[i] == '%' && i + 2 < cadena.size()) {
            auto digito = [](char c) -> int {
                if (c >= '0' && c <= '9') return c - '0';
                if (c >= 'a' && c <= 'f') return c - 'a' + 10;
                if (c >= 'A' && c <= 'F') return c - 'A' + 10;
                return -1;
            };
            int alto = digito(cadena[i + 1]), bajo = digito(cadena[i + 2]);
            if (alto >= 0 && bajo >= 0) {
                salida.push_back(static_cast<char>(alto * 16 + bajo));
                i += 2;
            } else salida.push_back(cadena[i]);
        } else salida.push_back(cadena[i]);
    }
    return salida;
}

std::map<std::string, std::string> camposFormulario(const std::string& texto) {
    std::map<std::string, std::string> valores;
    std::istringstream elementos(texto);
    std::string item;
    while (std::getline(elementos, item, '&')) {
        const auto igual = item.find('=');
        const auto clave = decodificar(item.substr(0, igual));
        if (!clave.empty()) valores[clave] = igual == std::string::npos ? "" : decodificar(item.substr(igual + 1));
    }
    return valores;
}

int leerEntero(const std::string& texto) {
    std::size_t longitud = 0;
    const int valor = std::stoi(texto, &longitud);
    if (longitud != texto.size()) throw std::invalid_argument("Parametro no numerico: " + texto);
    return valor;
}

ConfiguracionSimulacion interpretarConfiguracion(const std::string& cuerpo) {
    ConfiguracionSimulacion c;
    const auto campos = camposFormulario(cuerpo);
    const auto configurar = [&](const char* nombre, int& destino) {
        const auto encontrado = campos.find(nombre);
        if (encontrado != campos.end()) destino = leerEntero(encontrado->second);
    };
    configurar("vehicles", c.vehiculosPorEje);
    configurar("arrival", c.intervaloLlegadaMs);
    configurar("crossing", c.tiempoCruceMs);
    configurar("green", c.duracionVerdeMs);
    configurar("red", c.duracionTodoRojoMs);
    const auto modo = campos.find("mode");
    if (modo != campos.end()) {
        if (modo->second == "adaptive") c.adaptativo = true;
        else if (modo->second == "fixed") c.adaptativo = false;
        else throw std::invalid_argument("Modo desconocido: " + modo->second);
    }
    c.mostrarConsola = false;
    c.validar();
    return c;
}

std::string respuesta(int codigo, const std::string& tipo, const std::string& contenido) {
    const char* nombre = codigo == 200 ? "OK" : codigo == 400 ? "Bad Request" :
                         codigo == 404 ? "Not Found" : codigo == 409 ? "Conflict" :
                         codigo == 405 ? "Method Not Allowed" : "Internal Server Error";
    std::ostringstream out;
    out << "HTTP/1.1 " << codigo << ' ' << nombre << "\r\n"
        << "Content-Type: " << tipo << "\r\n"
        << "Content-Length: " << contenido.size() << "\r\n"
        << "Cache-Control: no-store\r\n"
        << "X-Content-Type-Options: nosniff\r\n"
        << "Content-Security-Policy: default-src 'self'; img-src 'self' data:; style-src 'self'; script-src 'self'; connect-src 'self'\r\n"
        << "Connection: close\r\n\r\n" << contenido;
    return out.str();
}

std::string jsonError(const std::string& mensaje) {
    return "{\"ok\":false,\"error\":\"" + MonitorSimulacion::escapar(mensaje) + "\"}";
}

bool recibirPeticion(Socket cliente, Peticion& peticion) {
    std::string buffer;
    char temporal[4096];
    std::size_t cabeceraFin = std::string::npos;
    while (buffer.size() < 65536) {
        const int leidos = recv(cliente, temporal, static_cast<int>(sizeof temporal), 0);
        if (leidos <= 0) return false;
        buffer.append(temporal, leidos);
        cabeceraFin = buffer.find("\r\n\r\n");
        if (cabeceraFin != std::string::npos) break;
    }
    if (cabeceraFin == std::string::npos) return false;
    std::istringstream cabecera(buffer.substr(0, cabeceraFin));
    std::string primerLinea;
    std::getline(cabecera, primerLinea);
    std::istringstream solicitud(primerLinea);
    solicitud >> peticion.metodo >> peticion.ruta;
    if (peticion.metodo.empty() || peticion.ruta.empty()) return false;
    std::size_t longitud = 0;
    std::string linea;
    while (std::getline(cabecera, linea)) {
        const auto dosPuntos = linea.find(':');
        if (dosPuntos == std::string::npos) continue;
        if (minusculas(linea.substr(0, dosPuntos)) == "content-length") {
            try { longitud = static_cast<std::size_t>(std::stoul(linea.substr(dosPuntos + 1))); }
            catch (...) { return false; }
        }
    }
    if (longitud > 16384) return false;
    peticion.cuerpo = buffer.substr(cabeceraFin + 4);
    while (peticion.cuerpo.size() < longitud) {
        const int leidos = recv(cliente, temporal, static_cast<int>(sizeof temporal), 0);
        if (leidos <= 0) return false;
        peticion.cuerpo.append(temporal, leidos);
    }
    peticion.cuerpo.resize(longitud);
    return true;
}

void enviar(Socket cliente, const std::string& dato) {
    std::size_t enviados = 0;
    while (enviados < dato.size()) {
        const int segmento = static_cast<int>(std::min<std::size_t>(dato.size() - enviados, 16384));
#ifdef MSG_NOSIGNAL
        const int resultado = send(cliente, dato.data() + enviados, segmento, MSG_NOSIGNAL);
#else
        const int resultado = send(cliente, dato.data() + enviados, segmento, 0);
#endif
        if (resultado <= 0) break;
        enviados += static_cast<std::size_t>(resultado);
    }
}
}

ServidorWeb::ServidorWeb(int puerto) : puerto_(puerto) {
    if (puerto < 1024 || puerto > 65535)
        throw std::invalid_argument("El puerto debe estar entre 1024 y 65535");
}

ServidorWeb::~ServidorWeb() {
    if (simulador_) simulador_->solicitarDetencion();
    if (hilo_.joinable()) hilo_.join();
}

void ServidorWeb::iniciarSimulacion(const ConfiguracionSimulacion& config) {
    if (hilo_.joinable()) hilo_.join();
    monitor_.iniciar(config);
    simulador_ = std::make_shared<Simulador>(config,
        [this](const EventoSimulacion& evento, const ResumenEstadisticas& resumen,
               const EstadoInterseccion& luces) {
            monitor_.registrar(evento, resumen, luces);
        });
    auto simulador = simulador_;
    hilo_ = std::thread([this, simulador, cantidad = config.vehiculosPorEje] {
        try {
            const auto resultado = simulador->ejecutar();
            // Interrumpido cuando no se atendieron todos los vehiculos previstos.
            monitor_.finalizar(resultado, resultado.totalAtendidos() < 2 * cantidad);
        } catch (const std::exception& e) {
            monitor_.fallar(e.what());
        } catch (...) {
            monitor_.fallar("Fallo inesperado del motor de simulacion");
        }
    });
}

int ServidorWeb::ejecutar() {
#ifdef _WIN32
    WSADATA datos;
    if (WSAStartup(MAKEWORD(2, 2), &datos) != 0)
        throw std::runtime_error("No se pudo iniciar Winsock");
#else
    signal(SIGPIPE, SIG_IGN);
#endif
    Socket servidor = socket(AF_INET, SOCK_STREAM, 0);
    if (servidor == SocketInvalido) throw std::runtime_error("Error al crear socket HTTP");
    int reutilizar = 1;
#ifdef _WIN32
    setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<const char*>(&reutilizar), sizeof reutilizar);
#else
    setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &reutilizar, sizeof reutilizar);
#endif
    sockaddr_in destino{};
    destino.sin_family = AF_INET;
    destino.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    destino.sin_port = htons(static_cast<unsigned short>(puerto_));
    if (bind(servidor, reinterpret_cast<sockaddr*>(&destino), sizeof destino) != 0 ||
        listen(servidor, 16) != 0) {
        cerrarSocket(servidor);
        throw std::runtime_error("No se pudo abrir el puerto " + std::to_string(puerto_) +
                                 ". Compruebe si ya esta ocupado.");
    }
    std::cout << "\nSimTrrafic | Interfaz web local\n"
              << "Abra http://127.0.0.1:" << puerto_ << " en su navegador\n"
              << "Servidor limitado a este equipo (127.0.0.1). Ctrl+C para salir.\n\n";

    for (;;) {
        Socket cliente = accept(servidor, nullptr, nullptr);
        if (cliente == SocketInvalido) continue;
#ifdef _WIN32
        DWORD timeout = 3000;
        setsockopt(cliente, SOL_SOCKET, SO_RCVTIMEO,
                   reinterpret_cast<const char*>(&timeout), sizeof timeout);
#else
        timeval timeout{3, 0};
        setsockopt(cliente, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);
#endif
        Peticion peticion;
        if (!recibirPeticion(cliente, peticion)) {
            enviar(cliente, respuesta(400, "application/json", jsonError("Peticion invalida")));
            cerrarSocket(cliente);
            continue;
        }
        std::string salida;
        const std::string& ruta = peticion.ruta;
        if (peticion.metodo == "GET" && ruta == "/api/status") {
            salida = respuesta(200, "application/json; charset=utf-8", monitor_.json());
        } else if (peticion.metodo == "POST" && ruta == "/api/start") {
            if (monitor_.ejecutando()) {
                salida = respuesta(409, "application/json", jsonError("La simulacion ya esta en marcha"));
            } else {
                try {
                    iniciarSimulacion(interpretarConfiguracion(peticion.cuerpo));
                    salida = respuesta(200, "application/json", "{\"ok\":true}");
                } catch (const std::exception& e) {
                    salida = respuesta(400, "application/json", jsonError(e.what()));
                }
            }
        } else if (peticion.metodo == "POST" && ruta == "/api/stop") {
            if (monitor_.ejecutando() && simulador_) {
                simulador_->solicitarDetencion();
                salida = respuesta(200, "application/json", "{\"ok\":true}");
            } else {
                salida = respuesta(409, "application/json", jsonError("No existe una ejecucion activa"));
            }
        } else if (peticion.metodo == "POST" && ruta == "/api/reset") {
            if (monitor_.ejecutando()) {
                salida = respuesta(409, "application/json", jsonError("Detenga la simulacion primero"));
            } else {
                if (hilo_.joinable()) hilo_.join();
                monitor_.reiniciarVista();
                salida = respuesta(200, "application/json", "{\"ok\":true}");
            }
        } else if (peticion.metodo == "GET" && (ruta == "/" || ruta == "/index.html" ||
                    ruta == "/styles.css" || ruta == "/app.js")) {
            const std::string nombre = (ruta == "/" ? "index.html" : ruta.substr(1));
            const auto archivo = leerArchivo(nombre);
            if (archivo.empty()) {
                salida = respuesta(404, "text/plain", "Recurso web no encontrado: " + nombre);
            } else {
                const char* tipo = nombre == "index.html" ? "text/html; charset=utf-8" :
                                  nombre == "styles.css" ? "text/css; charset=utf-8" :
                                  "text/javascript; charset=utf-8";
                salida = respuesta(200, tipo, archivo);
            }
        } else {
            salida = respuesta(404, "application/json", jsonError("Ruta no encontrada"));
        }
        enviar(cliente, salida);
        cerrarSocket(cliente);
    }
    return 0;
}
