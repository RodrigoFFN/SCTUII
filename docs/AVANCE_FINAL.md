# SimTrrafic 1.3.0 — resumen de la entrega

La interfaz anterior mezclaba una paleta clara con controles que heredaban fondos oscuros. Los campos numéricos apenas dejaban espacio para el valor completo y el mapa tenía cuatro manzanas prácticamente iguales. Esta entrega corrige esos problemas y reorganiza la aplicación alrededor de la intersección.

## Mejoras visuales y de uso

- Nueva barra superior oscura, superficies claras y acentos verdes, con estados y cifras legibles.
- Controles QML compartidos y personalizados, incluidos menús desplegables, campos numéricos, casillas, botones y barras de progreso.
- Campos que muestran hasta 10000 sin recortar el valor, con foco de teclado visible.
- Escena urbana con comercio, parque, plaza y viviendas; aceras, marcas viales, flechas y semáforos más claros.
- Autos y buses del dominio, colores por eje, detalles cenitales y separación de colas.
- Inicio del cruce desde la posición visual previa y separación también en la parte de la cola fuera de pantalla.
- Bitácora ajustable y plegable, seguimiento de eventos y exportación completa a CSV.
- Resultados compactos, total previsto actualizado al editar la cantidad y gráfico escalonado de eventos reales.
- Corrección de líneas finas del entorno que producían artefactos con renderizado de software al 150 %.

## Archivos nuevos

| Archivo | Propósito |
|---|---|
| `gui/qml/components/UiButton.qml` | Botones con estados explícitos de foco, pulsación y deshabilitado. |
| `gui/qml/components/UiSpinBox.qml` | Entrada numérica legible y editable. |
| `gui/qml/components/UiComboBox.qml` | Selector y menú con paleta propia. |
| `gui/qml/components/UiCheckBox.qml` | Casillas compactas y coherentes. |
| `gui/qml/components/UiProgressBar.qml` | Progreso sin fondos heredados del sistema. |
| `gui/qml/components/UrbanBlock.qml` | Manzanas urbanas reutilizables. |
| `tests/test_qt_interface.cpp` | Interacción con QML, escenarios, diálogos, tamaños y DPI. |
| `tests/test_vehicle_model.cpp` | Separación de colas, continuidad, tipos y reinicio. |
| `docs/captura_simulacion.png` | Captura real del escenario de tráfico normal. |

## Archivos modificados

| Archivos | Cambio |
|---|---|
| `gui/qml/Main.qml`, `Theme.qml` | Distribución, paleta completa, atajos y mensajes. |
| `gui/qml/components/ControlPanel.qml` | Agrupación de parámetros, controles y escenarios. |
| `gui/qml/components/TopBar.qml`, `StatisticsPanel.qml` | Jerarquía de información y resultados. |
| `gui/qml/components/EventLog.qml` | Plegado, navegación y exportación. |
| `gui/qml/components/TrafficScene.qml`, `TrafficLight.qml`, `Vehicle.qml` | Mapa, señalización y representación de vehículos. |
| `gui/VehicleModel.hpp`, `.cpp` | Dimensiones, continuidad y separación visual. |
| `gui/SimulationController.hpp`, `.cpp` | Mensajes de error, eje de eventos generales en CSV y tiempo de salida visual. |
| `CMakeLists.txt` | Nuevos recursos y pruebas; retirada de objetivos heredados. |
| `src/main.cpp` | Retirada de opciones del servidor HTTP. |
| `tests/test_qt_controller.cpp` | Carga máxima, rechazo de inicio simultáneo y fallo de escritura. |
| `README.md`, `docs/GUI_QT6.md`, `docs/AVANCE_FINAL.md` | Instrucciones, validación y cambios. |
| `docs/diagrama_clases.puml`, `docs/diseno_inicial.md` | Diagrama actual y referencia histórica actualizada. |

## Archivos eliminados

Después de verificar inicio, configuración, estadísticas, animación, parada, reinicio y exportación en Qt Quick:

- `gui/MainWindow.cpp`, `gui/MainWindow.hpp`, `gui/TrafficCanvas.cpp`, `gui/TrafficCanvas.hpp` y `gui/main_qt.cpp`.
- `include/ServidorWeb.hpp`, `src/ServidorWeb.cpp` y `tests/test_web_api.py`.
- `web/index.html`, `web/styles.css`, `web/app.js` y `docs/API_WEB.md`.

## Arquitectura

Se mantiene el motor C++17 y la comunicación directa mediante `SimulationController`, instantáneas tipadas y modelos de Qt. Qt Quick queda como única interfaz gráfica y la consola sigue compilando sin Qt. No se modificaron `src/Simulador.cpp` ni las clases del dominio: hilos, mutex, colas, políticas y exclusión mutua conservan su implementación original. Se mantuvieron las ocho pruebas del motor, incluida la serialización del monitor.

## Resultados de validación

- Compilación real en Debian 13, GCC 14.2, CMake 3.31.6 y Qt 6.8.2.
- **13/13 pruebas CTest aprobadas**: ocho del motor y cinco de Qt (modelo, controlador, integración, DPI y arranque).
- Configuración separada con `BUILD_QT_QUICK=OFF`: **8/8 pruebas aprobadas**, más una ejecución de consola con 4/4 vehículos atendidos.
- Escenarios exactos de la interfaz: flujo bajo **6/6**, flujo normal **12/12** y hora punta **32/32**, todos con cero pendientes.
- Carga máxima del controlador: **120/120** vehículos atendidos.
- Apertura y aceptación del diálogo QML de exportación, CSV con encabezado y todas las filas, y respuesta ante un destino inválido.
- Capturas inspeccionadas a 1280×720, 1366×768 y 1920×1080; comprobación adicional al 150 %.
- Pruebas gráficas repetidas después del ajuste final de trazos del mapa, sin errores QML.

Las capturas usan el renderizador de software de Qt. La animación está ligada a los tiempos y eventos del motor; los intervalos mínimos pueden ser más cortos que un ciclo de lectura. No se probó Windows, un kit Qt 6.2 ni el rendimiento de una GPU física. Esas verificaciones quedan pendientes y no se sustituyen por las pruebas de Linux.

## Commit recomendado

```text
feat(gui): rediseñar la interfaz Qt Quick y mejorar la visualización del tráfico
```
