# Avance integrado: SimTrrafic v1.0

## ¿Qué añadió esta entrega?

- Configuración validada, escenarios y modos fijo/adaptativo.
- Fábrica de vehículos con reglas centralizadas de creación.
- Reserva exclusiva del cruce y espera de despeje antes de cambiar de fase.
- Estadísticas de espera y API de observación del motor.
- Interfaz gráfica web en el equipo local que controla el motor C++.
- Inicio, parada y reinicio sin dejar hilos de ejecución sin unir.
- Pruebas unitarias y prueba de extremo a extremo de la API.
- Documentación de compilación, estructura y límites del prototipo.

## Demo para la exposición (5 a 7 minutos)

1. Mostrar `include/`, `src/`, `web/` y `tests/`, explicando las responsabilidades de objetos.
2. Ejecutar `cmake -S . -B build`, `cmake --build build -j2`, `ctest --test-dir build --output-on-failure`.
3. Ejecutar `./build/simtrrafic --gui` y abrir `http://127.0.0.1:8080`.
4. Elegir **Normal**, iniciar y mostrar colas, transiciones y resumen al finalizar.
5. Cambiar a **Hora punta**, ejecutar con estrategia adaptativa y señalar duración de verde variable en la bitácora.
6. Demostrar **Detener** y **Reiniciar**; recalcar que el motor libera los hilos.
7. Explicar diferencia entre la reserva crítica real del motor y la animación simplificada de la vista.

## Límites para informar al docente

El resultado es un prototipo académico concurrente completo para una intersección simplificada. No hay física, peatones, señal amarilla, múltiples cruces ni IA predictiva. La política adaptativa es heurística. No debe usarse para controlar señales reales.

## Futuras posibles mejoras

- Incluir semáforos amarillos y pasos peatonales como nuevas fases.
- Comparar tiempos medios fijo/adaptativo en conjuntos de escenarios experimentales.
- Exportar resultados a CSV y registrar percentiles de espera.
- Separar la interfaz como ejecutable opcional o incorporar una vista Qt6 si se requiere GUI nativa.
