# SimTrrafic (SIM-TRAFIC)

**Simulador Concurrente de Tráfico Urbano e Intersecciones Inteligentes**  
**Curso:** Tecnología de Objetos (TEO)  
**Versión base:** `v0.2.0` - con avance incremental de estadísticas

SimTrrafic es un proyecto académico en **C++17** que simula una intersección urbana
con hilos, vehículos y semáforos. El programa funciona en **consola**; todavía
no tiene interfaz gráfica.

## Funcionalidades actuales

- `ColaConcurrente<T>`: cola genérica con `mutex`, `condition_variable`, cierre y extracción bloqueante.
- `Vehiculo`: identificador y tipo (`Auto` o `Bus`).
- `Semaforo`: semáforo básico con luces roja y verde.
- `Interseccion`: coordina las luces de los ejes **Norte-Sur** y **Este-Oeste**
  para que nunca estén verdes simultáneamente.
- **Cinco hilos**: dos generadores, dos despachadores y un controlador de semáforos.
- Simulación de **12 vehículos** (6 por eje), con un intervalo en que ambas luces
  permanecen rojas entre fases.
- `Estadisticas`: registra vehículos generados y atendidos por eje mediante
  contadores atómicos (`std::atomic<int>`).
- Resumen al terminar: generados, atendidos, pendientes y desglose por eje.
- Pruebas para la cola concurrente y para la exclusión de fases de la intersección.

> Alcance: este es un prototipo de sincronización de semáforos, no un simulador de
> seguridad vial. El paso se autoriza cuando se observa luz verde; aún no se
> modelan ocupación física del cruce, colisiones, tiempos configurables ni peatones.

## Requisitos

- Compilador con soporte para C++17 (GCC, Clang o MSVC moderno).
- CMake 3.16 o superior (recomendado).
- Soporte de hilos de la biblioteca estándar.
- **No hace falta SFML** para ejecutar esta versión.

## Compilar y probar con CMake

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Para ejecutar la simulación:

- Linux: `./build/simtrrafic`
- Windows (Visual Studio): `build\\Debug\\simtrrafic.exe`

La ruta del ejecutable puede cambiar según el generador de CMake.

## Compilar con g++ sin CMake

```bash
g++ -std=c++17 -Wall -Wextra -pthread -Iinclude src/main.cpp -o simtrrafic
./simtrrafic

g++ -std=c++17 -Wall -Wextra -pthread -Iinclude tests/test_concurrencia.cpp -o test_concurrencia
./test_concurrencia

g++ -std=c++17 -Wall -Wextra -pthread -Iinclude tests/test_interseccion.cpp -o test_interseccion
./test_interseccion
```

## Estructura del repositorio

```text
SimTrrafic/
├── CMakeLists.txt
├── README.md
├── .gitignore
├── include/
│   ├── ColaConcurrente.hpp
│   ├── Estadisticas.hpp      # Nuevo: contadores seguros entre hilos
│   ├── Interseccion.hpp        # Nuevo: coordinación de dos ejes
│   ├── Semaforo.hpp
│   └── Vehiculo.hpp
├── src/
│   └── main.cpp                # Simulación de dos ejes y resumen final
├── docs/
│   └── diseno_inicial.md
└── tests/
    ├── test_concurrencia.cpp
    └── test_interseccion.cpp  # Nuevo: prueba de exclusión de fases
```

## Estadísticas del tercer avance

`Estadisticas` registra cada vehículo **después de insertarlo correctamente**
en su cola y registra su atención **al concluir el paso simulado**.
Los contadores atómicos evitan condiciones de carrera entre productores y
despachadores. El resumen se consulta una vez que todos los hilos han terminado.

Al final de la ejecución se imprime un resumen similar al siguiente:

```text
========== RESUMEN DE SIMULACION ==========
Vehiculos generados: 12
Vehiculos atendidos: 12
Norte-Sur: 6 atendidos de 6 generados
Este-Oeste: 6 atendidos de 6 generados
Vehiculos pendientes: 0
===========================================
```

Estas cifras corresponden a la configuración actual de seis vehículos por eje;
no representan el tiempo de espera ni la ocupación física de la intersección.

## Próximos avances

1. Crear clases `Carril` y `Simulador` para separar las responsabilidades del `main`.
2. Introducir tipos de vehículos mediante **Factory Method**.
3. Implementar **State** para las transiciones de luces y **Observer** para publicar eventos.
4. Incorporar peatones y una representación 2D con SFML.

Los patrones y la GUI son **trabajo pendiente**, no funciones ya implementadas.
En `docs/diseno_inicial.md` se describe la arquitectura y las limitaciones.
