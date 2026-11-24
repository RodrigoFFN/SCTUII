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
4. Pulsa **Iniciar simulación**. Los **autos y buses se desplazan** al llegar,
   se acercan a la línea de detención, esperan en su eje, cruzan cuando el
   motor reserva la intersección y salen de la vista tras terminar el cruce.
   Los semáforos, estadísticas y 80 eventos recientes se actualizan en vivo.
5. Pulsa **Detener** para interrumpir el motor de forma cooperativa.
6. Pulsa **Reiniciar panel** cuando termine, o inicia otra simulación.
7. Exporta a CSV los eventos que actualmente aparecen en la bitácora.

**Nota:** los movimientos son interpolaciones 2D ilustrativas; no se calculan
coordenadas físicas, aceleraciones ni distancias reales. **Las llegadas, entradas
al cruce y salidas sí corresponden a eventos auténticos del motor C++**. Los
coches visibles representan el eje N/S en sentido descendente y el E/O hacia
la derecha para facilitar la lectura del dibujo. `occupied` indica la reserva
real de la zona crítica.

## Ver movimiento claramente

El escenario **Normal** inicia con 8 vehículos por eje, una llegada cada 180 ms,
cruces de 420 ms y verde de 1800 ms. Son valores deliberadamente visibles para
una demostración, no los valores predeterminados del motor en consola. Para
observar una cola más larga durante una exposición, prueba esta configuración:

- Escenario **Personalizado**, **12 vehículos por eje**.
- **Llegada: 200 ms**, **cruce: 600 ms**.
- **Verde: 2800 ms**, **ambos rojo: 200 ms**.
- Mantén marcada **Animación fluida (60 FPS)**.

Puedes cambiar los parámetros en cualquier momento entre ejecuciones. El visor
adapta la escala al espacio disponible; si tu pantalla es pequeña, maximiza la
ventana o usa el desplazamiento vertical del panel.

La cola muestra los vehículos cercanos al cruce, hasta cuatro de N/S y cinco de
E/O. Los contadores de vehículos restantes siguen siendo los del motor.

## Diseño y concurrencia

- `gui/TrafficCanvas.*`: dibuja y anima el cruce con `QPainter` y `QTimer`.
- `gui/TrafficAnimationModel.hpp`: reconstruye cada coche a partir de los
  eventos `llegada`, `cruce_inicio` y `paso` sin depender de Qt.
- `gui/MainWindow.*`: componentes, estilos, parámetros y estadísticas de Qt.
- `gui/main_qt.cpp`: inicio de `QApplication`.
- `Simulador`: continúa en su propio hilo `std::thread`.
- `MonitorSimulacion`: receptor thread-safe de los callbacks del motor.
- `QTimer` (75 ms): obtiene una instantánea del monitor, la interpreta en el
  **hilo de la GUI** y actualiza los widgets. Otro `QTimer` de 16 ms repinta
  los coches entre eventos para obtener movimiento fluido. Ningún hilo del
  motor modifica Qt.
- Al cerrar la ventana se solicita detención y se hace `join` al trabajador.
- La ejecución de escritorio **no inicia servidor HTTP**.

La vista no requiere `Q_OBJECT`, moc, QML, Qt Quick o dependencias adicionales.

## Pruebas

La compilación agrega `interfaz_qt_smoke` a CTest (con `QT_QPA_PLATFORM=offscreen`).
Comprueba que `QApplication` y `MainWindow` pueden iniciarse y cerrar sin pantalla.
El test `animacion_eventos` verifica el modelo de coches sin Qt y
`eventos_cruce_reales` comprueba que el evento de entrada solamente se emite
tras reservar la zona crítica de la intersección.

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
