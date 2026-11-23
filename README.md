# SimTrrafic (SIM-TRAFIC)

**Simulador Concurrente de Tráfico Urbano e Intersecciones Inteligentes**  
**Curso:** Tecnología de Objetos (TEO)  
**Versión:** `v0.3.0` - tercer avance (estadísticas y separación en `Carril` y `Simulador`)

SimTrrafic es un proyecto académico en **C++17** que simula una intersección urbana
con hilos, vehículos y semáforos. El programa funciona en **consola**; todavía
no tiene interfaz gráfica.

## Funcionalidades actuales

- `ColaConcurrente<T>`: cola genérica con `mutex`, `condition_variable`, cierre y extracción bloqueante.
- `Vehiculo`: identificador y tipo (`Auto` o `Bus`).
- `Semaforo`: semáforo básico con luces roja y verde.
- `Interseccion`: coordina las luces de los ejes **Norte-Sur** y **Este-Oeste**
  para que nunca estén verdes simultáneamente.
- `Carril`: encapsula la cola de vehículos de un eje y conoce su dirección.
- `Simulador`: crea y coordina los hilos, y devuelve el resumen al terminar.
  Los hilos arrancan juntos mediante una barrera (`promise`/`shared_future`); si
  falla la creación de alguno, los demás salen sin ejecutar su tarea y se relanza
  la excepción.
- **Cinco hilos**: dos generadores, dos despachadores y un controlador de semáforos.
- Simulación de **12 vehículos** (6 por eje; los ids múltiplos de 3 son `Bus`), con un
  intervalo en que ambas luces permanecen rojas entre fases.
- `Estadisticas`: registra vehículos generados y atendidos por eje mediante
  contadores atómicos (`std::atomic<int>`).
- Resumen al terminar: generados, atendidos, pendientes y desglose por eje.
- Pruebas para la cola concurrente y para la exclusión de fases de la intersección.

> Alcance: este es un prototipo de sincronización de semáforos, no un simulador de
> seguridad vial. El paso se autoriza cuando se observa luz verde; aún no se
> modelan ocupación física del cruce, colisiones, tiempos configurables ni peatones.

## Requisitos

- Compilador con soporte completo de C++17 e hilos de la biblioteca estándar:
  **GCC 7 o superior**, Clang o MSVC moderno.
- En Windows con MinGW, usar una distribución actual con hilos POSIX (por ejemplo
  **MSYS2 UCRT64**). Los MinGW antiguos con hilos `win32` no incluyen `std::mutex`
  ni `<optional>` y la compilación falla.
- CMake 3.16 o superior (recomendado).
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

La simulación necesita `src/Simulador.cpp` además de `src/main.cpp`:

```bash
g++ -std=c++17 -Wall -Wextra -pthread -Iinclude src/main.cpp src/Simulador.cpp -o simtrrafic
./simtrrafic
```

Las pruebas solo usan los encabezados de `include/`:

```bash
g++ -std=c++17 -Wall -Wextra -pthread -Iinclude tests/test_concurrencia.cpp -o test_concurrencia
./test_concurrencia

g++ -std=c++17 -Wall -Wextra -pthread -Iinclude tests/test_interseccion.cpp -o test_interseccion
./test_interseccion
```

## Estructura del repositorio

```text
SimTrrafic/
├── CMakeLists.txt              # Biblioteca simtrrafic_core + ejecutable + pruebas
├── README.md
├── .gitignore
├── include/
│   ├── Carril.hpp              # Cola de vehículos de un eje
│   ├── ColaConcurrente.hpp
│   ├── Estadisticas.hpp        # Contadores seguros entre hilos
│   ├── Interseccion.hpp        # Coordinación de dos ejes
│   ├── Semaforo.hpp
│   ├── Simulador.hpp           # Interfaz del coordinador de hilos
│   └── Vehiculo.hpp
├── src/
│   ├── main.cpp                # Crea el Simulador y lo ejecuta
│   └── Simulador.cpp           # Hilos, fases de semáforo y resumen final
├── docs/
│   └── diseno_inicial.md
└── tests/
    ├── test_concurrencia.cpp
    └── test_interseccion.cpp   # Prueba de exclusión de fases
```

## Cómo funciona la ejecución

Al ejecutar, la consola muestra tres tipos de línea:

- `[LLEGADA]`: un generador inserta un vehículo en su carril.
- `[SEMAFOROS]`: el controlador cambia de fase. Entre una fase y otra los dos
  ejes pasan por ROJO.
- `[PASO]`: un despachador deja cruzar un vehículo del eje que tiene luz verde.

## Estadísticas del tercer avance

`Estadisticas` registra cada vehículo **después de insertarlo correctamente**
en su carril y registra su atención **al concluir el paso simulado**.
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

1. Introducir tipos de vehículos mediante **Factory Method** (jerarquía `Vehiculo`, `Auto`, `Bus`).
2. Implementar **State** para las transiciones de luces y **Observer** para publicar eventos.
3. Incorporar peatones y una representación 2D con SFML.
4. Ampliar las pruebas a `Carril`, `Estadisticas` y `Simulador`.

Los patrones y la GUI son **trabajo pendiente**, no funciones ya implementadas.
En `docs/diseno_inicial.md` se describe la arquitectura y las limitaciones.
