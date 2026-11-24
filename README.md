# SimTrrafic

**Simulador concurrente de tráfico urbano**  
Tecnología de Objetos · C++17 · Qt 6

SimTrrafic simula el tránsito de vehículos en una intersección con dos ejes: Norte–Sur y Este–Oeste. La generación de vehículos, las colas de espera y el control de semáforos se ejecutan de forma concurrente.

El mismo motor de simulación se utiliza en la aplicación de escritorio desarrollada con Qt 6, el panel web local y la versión de consola.

## Funcionalidades

- Generación de autos y buses en ambos ejes.
- Colas concurrentes para gestionar los vehículos en espera.
- Semáforos con ciclos fijos o ajuste adaptativo según la cantidad de vehículos pendientes.
- Fase de seguridad con ambos semáforos en rojo y control de acceso a la zona de cruce.
- Configuración de cantidad de vehículos y tiempos de llegada, cruce y semáforos.
- Estadísticas de vehículos generados, atendidos y pendientes, además de tiempos de espera estimados.
- Interfaz Qt 6 con vehículos animados, estado de los semáforos, controles y registro de eventos.
- Panel web local y ejecución por consola.
- Pruebas automatizadas con CTest.

## Requisitos

- Compilador compatible con **C++17** (GCC, Clang o MSVC).
- **CMake 3.16** o superior.
- **Qt 6.2** o superior, con el módulo Widgets, para la aplicación de escritorio.
- **Python 3** para la prueba automatizada de la API web (opcional).

El motor, la consola y el panel web no requieren Qt. La compilación se realiza con CMake en Windows, Linux o macOS, siempre que las dependencias estén instaladas.

## Compilación y ejecución

### Interfaz de escritorio (Qt 6)

Desde la carpeta del proyecto:

```bash
cmake -S . -B build-qt -DBUILD_QT_GUI=ON
cmake --build build-qt --config Release --parallel
```

Si CMake no detecta Qt, se puede indicar su instalación mediante `CMAKE_PREFIX_PATH`:

```bash
cmake -S . -B build-qt -DBUILD_QT_GUI=ON -DCMAKE_PREFIX_PATH="RUTA_DE_QT"
```

Ejecutar en Linux o macOS con un generador de una sola configuración:

```bash
./build-qt/simtrrafic_qt
```

En Windows con un generador de varias configuraciones, por ejemplo Visual Studio:

```powershell
.\build-qt\Release\simtrrafic_qt.exe
```

La ubicación del ejecutable depende del generador de CMake utilizado.

### Consola y panel web

```bash
cmake -S . -B build
cmake --build build --config Release --parallel
```

Iniciar la simulación por consola:

```bash
./build/simtrrafic
```

Iniciar el panel web:

```bash
./build/simtrrafic --gui
```

Abrir [http://127.0.0.1:8080](http://127.0.0.1:8080) en el navegador. Para utilizar otro puerto:

```bash
./build/simtrrafic --gui --port 8081
```

En Windows, utiliza el ejecutable `simtrrafic.exe` generado por CMake; con Visual Studio suele estar en `build\Release\`.

## Parámetros de la simulación

| Parámetro | Descripción | Predeterminado |
| --- | --- | ---: |
| `--vehiculos N` | Vehículos por eje | 6 |
| `--llegada-ms N` | Intervalo entre llegadas | 90 ms |
| `--cruce-ms N` | Tiempo de cruce por vehículo | 130 ms |
| `--verde-ms N` | Duración base de la luz verde | 1000 ms |
| `--rojo-ms N` | Duración de la fase con ambos semáforos en rojo | 220 ms |
| `--adaptativo` | Ajuste del verde según la demanda | Desactivado |
| `--fijo` | Ciclo semafórico de duración fija | Activado |
| `--help` | Lista de opciones disponibles | — |

Ejemplos:

```bash
./build/simtrrafic --vehiculos 10 --verde-ms 1500
./build/simtrrafic --vehiculos 12 --adaptativo
```

En la aplicación Qt, estos valores también pueden modificarse desde el panel de configuración antes de iniciar la simulación.

## Estructura del proyecto

```text
SCTUII/
├── include/          # Clases y estructuras del simulador
├── src/              # Motor, consola y servidor web
├── gui/              # Interfaz de escritorio Qt 6
├── web/              # Interfaz web (HTML, CSS y JavaScript)
├── tests/            # Pruebas automatizadas
├── docs/             # Documentación técnica
├── CMakeLists.txt
└── README.md
```

### Componentes principales

| Componente | Función |
| --- | --- |
| `Vehiculo`, `FabricaVehiculos` | Representación y creación de vehículos. |
| `ColaConcurrente`, `Carril` | Gestión de vehículos pendientes en cada eje. |
| `Semaforo`, `Interseccion` | Control de luces y exclusión mutua del cruce. |
| `ConfiguracionSimulacion` | Parámetros y validación de entradas. |
| `PoliticaSemaforo` | Duración de fases fijas o adaptativas. |
| `Estadisticas` | Registro de vehículos y tiempos de espera. |
| `Simulador` | Coordinación de hilos y ejecución. |
| `Eventos`, `MonitorSimulacion` | Publicación y consulta del estado de la simulación. |
| `ServidorWeb` | Servidor HTTP para el panel local. |

La simulación trabaja con dos generadores de vehículos, dos despachadores y un hilo de control de semáforos. Se utilizan `std::thread`, `std::mutex`, `std::condition_variable` y operaciones atómicas para coordinar el acceso a los datos compartidos.

## Pruebas

Para compilar y ejecutar las pruebas:

```bash
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

Para incluir las pruebas de la aplicación Qt:

```bash
cmake -S . -B build-qt -DBUILD_QT_GUI=ON -DBUILD_TESTING=ON
cmake --build build-qt --config Release --parallel
ctest --test-dir build-qt -C Release --output-on-failure
```

## Documentación

- [Diseño inicial](docs/diseno_inicial.md)
- [Interfaz Qt 6](docs/GUI_QT6.md)
- [API y panel web](docs/API_WEB.md)
- [Diagrama de clases](docs/diagrama_clases.puml)

## Alcance

La simulación representa una intersección simplificada. Los vehículos de la interfaz se animan a partir de eventos del motor; no se calcula movimiento físico real. La política adaptativa ajusta las fases según la demanda observada, sin utilizar un modelo de aprendizaje automático.
