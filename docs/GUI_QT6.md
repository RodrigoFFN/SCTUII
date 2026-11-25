# Interfaz Qt Quick: implementación y validación

La interfaz se carga desde recursos locales y usa el estilo Basic de Qt como base. `UiButton`, `UiSpinBox`, `UiComboBox`, `UiCheckBox` y `UiProgressBar` definen sus fondos, contenido, bordes y estados de foco. La paleta de `Main.qml` también cubre los controles auxiliares, evitando la mezcla de texto oscuro con fondos del tema oscuro del sistema.

## Decisiones de presentación

- Barra superior compacta, parámetros a la izquierda y resultados a la derecha. El espacio central crece con la ventana.
- `SplitView` separa mapa y bitácora. La bitácora puede plegarse, y los paneles laterales tienen desplazamiento cuando falta altura.
- El valor numérico dispone de espacio para cinco dígitos. Las unidades quedan junto a la etiqueta, y no compiten con la cifra.
- `UrbanBlock` dibuja comercio, parque, plaza y viviendas con elementos nativos de Qt Quick. El entorno no se redibuja mediante Canvas en cada fotograma.
- Los autos y buses se componen de carrocería, cristales, techo, ruedas y luces. Se conservan sus tipos reales.
- La curva de atención es escalonada: sube al recibir cada evento de paso, con su tiempo registrado. El pequeño Canvas del gráfico se repinta al cambiar el historial o su tamaño.
- Los eventos se insertan en un `QAbstractListModel`; `ListView` reutiliza delegados y muestra el evento más reciente primero.

## Integración

El motor concurrente no cambió. `SimulationController` adapta sus instantáneas a propiedades QML y modelos Qt. `VehicleModel` solo calcula posiciones de dibujo: no cambia fases, no atiende vehículos y no decide quién cruza.

El comienzo del cruce conserva la posición visual anterior, la cola mantiene separación incluso fuera del mapa y la salida utiliza una velocidad visual común por eje para evitar que un auto alcance a un bus por su menor longitud. Las etapas `waiting`, `crossing` y `served` proceden del monitor. Los vehículos que no caben se contabilizan en el indicador de cola fuera de vista.

El CSV usa `QSaveFile`, UTF-8 y el historial completo. Los eventos generales se exportan con eje «—», igual que en la bitácora. Los errores y confirmaciones tienen colores distintos.

## Pruebas reproducibles

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j4
ctest --test-dir build --output-on-failure
```

Para guardar capturas durante la prueba de interfaz en Linux:

```bash
SIMTRAFFIC_CAPTURE_DIR="$PWD/capturas" \
QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software \
./build/test_qt_interface
```

Para revisar solo distribución, edición numérica, foco, plegado y movimiento reducido:

```bash
QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software \
QT_SCALE_FACTOR=1.5 ./build/test_qt_interface --layout-only
```

| Comprobación | Cobertura |
|---|---|
| Arranque sin ejecutar | Contadores y modelos vacíos; carga de QML y recursos. |
| Flujo bajo, normal e intenso | Selección mediante ComboBox y clic en iniciar; 6, 12 y 32 atendidos. |
| Política fija/adaptativa | Bajo y normal fijos; hora punta adaptativa; ambas también en el controlador. |
| Colas, cruce y semáforos | Fases de ambos ejes, ocupación y pendientes durante las ejecuciones. |
| Tipos y separación | Prueba de `VehicleModel`, con autos y buses y colas que exceden el mapa. |
| Continuidad y eventos | Cruce desde la posición anterior; el reloj visual no inventa un evento de paso. |
| Detención y reinicio | Botones QML; modelos vacíos al reiniciar y nueva ejecución. |
| Finalización | Total atendido y curva coherentes, cero pendientes. |
| CSV | Apertura del diálogo y señal de aceptación, fichero completo y error de escritura. |
| Teclado | Edición de 10000, confirmación con Tab y cambio de foco. |
| Ventanas | 1280×720, 1366×768 y 1920×1080; capturas inspeccionadas. |
| DPI | Escala del 150 % con prueba de distribución. |
| Cierre con motor activo | Cierre de ventana y destructor del controlador, cancelación y unión de hilos. |
| Carga máxima | 60 vehículos por eje: 120 atendidos, sin filas de otra ejecución. |
| Consola independiente | Configuración sin Qt, compilación y ocho pruebas del motor. |

**Resultado: 13/13 pruebas CTest aprobadas; 8/8 en la compilación independiente de consola.**

Entorno: Debian 13, GCC 14.2, CMake 3.31.6 y Qt 6.8.2; renderizado de software sin pantalla física. La prueba de interacción falla si el motor QML emite advertencias. El proceso gráfico permanece activo durante las simulaciones y procesa acciones de usuario.

Queda pendiente la ejecución en Windows, un kit Qt 6.2 y la medición de fluidez con aceleración gráfica real. No se afirma que estas comprobaciones se hayan realizado. Los tiempos del motor dependen del planificador del sistema operativo; no son resultados deterministas de rendimiento.

## Referencias técnicas

- [Personalización de Qt Quick Controls, Qt 6.2](https://doc.qt.io/qt-6.2/qtquickcontrols2-customize.html).
- [FileDialog de Qt Quick Dialogs](https://doc.qt.io/qt-6/qml-qtquick-dialogs-filedialog.html).
