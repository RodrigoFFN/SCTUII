# SimTrrafic — Simulador concurrente de tráfico

**Curso:** Tecnología de Objetos (TEO) · **Lenguaje:** C++17 · **Versión:** 1.1.0

SimTrrafic simula una intersección de dos ejes, con vehículos, carriles y semáforos coordinados en **cinco hilos**. Esta versión incorpora una **aplicación de escritorio Qt 6 Widgets** y una **interfaz gráfica web local** (abre el panel en tu navegador), configuración de parámetros, modo fijo/adaptativo, eventos en tiempo real, estadísticas y pruebas automatizadas.

> Se conserva la base de C++ del proyecto del equipo (`Carril`, `ColaConcurrente`, `Vehiculo`, `Semaforo`, `Interseccion`, `Estadisticas` y `Simulador`). La consola y la interfaz web siguen funcionando **sin Qt**. La aplicación de escritorio requiere Qt 6 Widgets, pero no necesita npm ni internet.

## Inicio rápido con Qt 6 en CachyOS (Linux)

```bash
sudo pacman -S --needed base-devel cmake gcc qt6-base
cmake -S . -B build-qt -DCMAKE_BUILD_TYPE=Debug -DBUILD_QT_GUI=ON
cmake --build build-qt -j2
ctest --test-dir build-qt --output-on-failure
./build-qt/simtrrafic_qt
```

**Qt 6 muestra una ventana de escritorio**, no un navegador. Incluye controles de ejecución, escenarios, intersección animada esquemáticamente, semáforos y eventos reales. El modo web anterior sigue disponible con `./build-qt/simtrrafic --gui`.

Para ejecutar una comprobación automática de apertura de ventana sin pantalla:

```bash
QT_QPA_PLATFORM=offscreen ./build-qt/simtrrafic_qt --smoke-test
```

Más detalles en [`docs/GUI_QT6.md`](docs/GUI_QT6.md).

## Inicio rápido en CachyOS (Linux) — consola y panel web

```bash
sudo pacman -S --needed cmake gcc python
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j2
ctest --test-dir build --output-on-failure
./build/simtrrafic --gui
```

**Abre <http://127.0.0.1:8080> en Firefox, Chromium o cualquier navegador moderno.** El servidor se limita a la interfaz loopback de este equipo; no acepta conexiones de otros dispositivos. Para terminarlo, pulsa `Ctrl+C` en la terminal.

Si el puerto está ocupado:

```bash
./build/simtrrafic --gui --port 8081
```

En Windows (MSVC o MSYS2 UCRT64), compila con CMake y ejecuta `simtrrafic.exe --gui`; la ubicación puede ser `build/Debug/`. El servicio HTTP utiliza Winsock en Windows. La validación automatizada de esta entrega se realizó en Linux.

## Qué puedes hacer en la interfaz

1. **Iniciar, detener y reiniciar** una simulación.
2. Elegir escenario rápido: **flujo bajo, normal u hora punta**.
3. Configurar vehículos por eje, intervalo de llegadas, tiempo de cruce, fase verde y transición todo rojo.
4. Cambiar entre **ciclo fijo** y **estrategia adaptativa** (ajuste heurístico a partir de la cantidad pendiente en cada eje).
5. Observar estados de semáforos, vehículos en cola aproximados, contadores por eje, tiempo de espera medio y bitácora.
6. Mostrar gráficamente la evolución de vehículos atendidos, con actualización periódica desde C++.

La representación vial es **ilustrativa**: no reproduce trayectorias físicas ni una ciudad real. El motor sí impide que dos vehículos ocupen simultáneamente su zona crítica simulada, y exige un tiempo de despeje antes de autorizar la siguiente fase.

## Consola (sin interfaz)

```bash
./build/simtrrafic
./build/simtrrafic --vehiculos 10 --llegada-ms 70 --cruce-ms 120 --verde-ms 900 --rojo-ms 200
./build/simtrrafic --vehiculos 12 --adaptativo
./build/simtrrafic --help
```

### Valores y límites

| Opción | Predeterminado | Valores admitidos |
| --- | ---: | ---: |
| `--vehiculos` | 6 por eje | 1–60 |
| `--llegada-ms` | 90 | 10–5000 ms |
| `--cruce-ms` | 130 | 20–5000 ms |
| `--verde-ms` | 1000 | 100–10000 ms |
| `--rojo-ms` | 220 | 20–5000 ms |
| `--adaptativo` / `--fijo` | fijo | dos estrategias |
| `--gui` | desactivado | interfaz en navegador |
| `--port` | 8080 | 1024–65535 (con `--gui`) |

En modo adaptativo la duración cambia en función de la diferencia de vehículos pendientes entre ambos ejes, dentro de un rango acotado. **Es una regla heurística educativa, no un algoritmo de IA entrenado ni un sistema para semáforos reales.**

## Arquitectura de objetos

| Clase o módulo | Responsabilidad |
| --- | --- |
| `Vehiculo` + `FabricaVehiculos` | Representar y construir autos y buses. |
| `ColaConcurrente<T>` + `Carril` | Almacenar vehículos de forma segura entre hilos. |
| `Semaforo` + `Interseccion` | Sincronizar luces y reservar la zona crítica de cruce. |
| `PoliticaSemaforo` | Calcular duración de fase en modo fijo o adaptativo. |
| `ConfiguracionSimulacion` | Encapsular y validar los parámetros de ejecución. |
| `Estadisticas` | Registrar contadores y espera aproximada con operaciones atómicas. |
| `Simulador` | Coordinar generadores, despachadores, controlador, parada y eventos. |
| `Eventos` + `MonitorSimulacion` | Desacoplar el motor de los consumidores de información, producir instantáneas JSON. |
| `ServidorWeb` | Exponer el panel y la API HTTP **solo en localhost**. |
| `web/` | Vista HTML/CSS/JS que consulta el estado y envía acciones. |

El sistema utiliza una **fábrica estática** para tipos de vehículos y una **interfaz de observador** mediante callback; no se afirma que implemente las versiones completas de Factory Method, State u Observer de GoF.

## Concurrencia y seguridad

- Se lanzan cinco hilos: generador NS, generador EO, despachador NS, despachador EO y controlador de semáforos.
- Las colas utilizan `std::mutex`, `std::condition_variable` y un protocolo de cierre.
- La intersección pone ambos semáforos en rojo antes de activar un eje; **espera a que el cruce previo quede desocupado**.
- Para permitir detener una ejecución, el motor notifica a los hilos, cancela esperas, cierra las colas y hace `join`.
- Los contadores son atómicos y las instantáneas del panel están protegidas con `mutex`.

## Pruebas

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j2
ctest --test-dir build --output-on-failure
```

La suite incluye pruebas de colas, intersección, validación, fábrica, exclusión del cruce, política adaptativa, integración del simulador, monitor JSON y API HTTP local. La prueba HTTP requiere Python 3: CMake la registra solo cuando encuentra el intérprete.

## Estructura del proyecto

```text
SCTUII/
├── CMakeLists.txt
├── README.md
├── include/                # clases del dominio, configuracion, monitor y servidor
├── src/                    # Simulador.cpp, ServidorWeb.cpp y main.cpp
├── gui/                    # interfaz de escritorio Qt 6 Widgets
├── web/                    # index.html, styles.css, app.js
├── tests/                  # pruebas unitarias y prueba HTTP end-to-end
└── docs/                   # diseno, arquitectura y guia de exposicion
```

## Alcance y límites

La versión implementa una **intersección bidireccional simplificada**. No tiene peatones, múltiples intersecciones, mapas geográficos, vehículos con dinámica física, semáforos amarillos ni decisiones con aprendizaje automático. Las esperas son **estimaciones de simulación**, no mediciones del mundo real. La vista muestra vehículos agregados por eje y anima cruces de forma esquemática.

Consulta `docs/AVANCE_FINAL.md` para ver la división por tareas y la demostración sugerida, y `docs/API_WEB.md` para las rutas de la interfaz.
