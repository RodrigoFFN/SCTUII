# SimTrrafic | Interfaz de escritorio Qt 6 Widgets

## Objetivo

Añadir una ventana de escritorio nativa en Qt 6 a la simulación concurrente ya
existente. El motor C++17 continúa siendo el mismo para la consola, el panel
web y el escritorio. La vista **no genera datos ficticios**: recibe eventos y
estadísticas reales del objeto `Simulador`.

## Dependencias

En CachyOS / Arch Linux:

```bash
sudo pacman -S --needed base-devel cmake gcc qt6-base
```

## Compilar

```bash
cmake -S . -B build-qt -DCMAKE_BUILD_TYPE=Debug -DBUILD_QT_GUI=ON
cmake --build build-qt -j2
ctest --test-dir build-qt --output-on-failure
```

## Ejecutar

```bash
./build-qt/simtrrafic_qt
```

En el escritorio:

1. Elige un escenario: normal, tráfico ligero u hora punta.
2. Ajusta los vehículos por eje y los tiempos (en milisegundos).
3. Selecciona el tipo de semáforos (fijos o adaptativos).
4. Pulsa **Iniciar simulación**. La ventana presenta el estado de ambos
   semáforos, las colas aproximadas, estadísticas y los 80 eventos más recientes.
5. Pulsa **Detener** para interrumpir el motor de forma cooperativa.
6. Pulsa **Reiniciar panel** cuando termine, o inicia otra simulación.
7. Exporta a CSV los eventos que actualmente aparecen en la bitácora.

**Nota:** el gráfico de vehículos es ilustrativo; el motor no tiene coordenadas
físicas. El estado `occupied` indica reserva del cruce en el motor.

## Diseño y concurrencia

- `gui/TrafficCanvas.*`: dibuja el cruce con `QPainter`.
- `gui/MainWindow.*`: componentes, estilos, parámetros y estadísticas de Qt.
- `gui/main_qt.cpp`: inicio de `QApplication`.
- `Simulador`: continúa en su propio hilo `std::thread`.
- `MonitorSimulacion`: receptor thread-safe de los callbacks del motor.
- `QTimer` (75 ms): obtiene una instantánea del monitor, la interpreta en el
  **hilo de la GUI** y actualiza los widgets. Ningún hilo del motor modifica Qt.
- Al cerrar la ventana se solicita detención y se hace `join` al trabajador.
- La ejecución de escritorio **no inicia servidor HTTP**.

La vista no requiere `Q_OBJECT`, moc, QML, Qt Quick o dependencias adicionales.

## Pruebas

La compilación agrega `interfaz_qt_smoke` a CTest (con `QT_QPA_PLATFORM=offscreen`).
Comprueba que `QApplication` y `MainWindow` pueden iniciarse y cerrar sin pantalla.

```bash
QT_QPA_PLATFORM=offscreen ./build-qt/simtrrafic_qt --smoke-test
```

## Versiones sin Qt

La construcción predeterminada mantiene solo la consola y el panel web:

```bash
cmake -S . -B build
cmake --build build -j2
./build/simtrrafic --gui
```

Si pides `-DBUILD_QT_GUI=ON` y Qt 6 no está instalado, CMake informa el error
y no simula que haya construido la GUI.
