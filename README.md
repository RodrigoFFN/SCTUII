# SimTrrafic (SIM-TRAFIC)

**Simulador Concurrente de Trafico Urbano e Intersecciones Inteligentes**  
**Curso:** Tecnologia de Objetos (TEO)  
**Version:** `v0.1.0` - primer avance

SimTrrafic es un proyecto academico en C++17 para simular una interseccion urbana,
con vehiculos, semaforos y concurrencia. Este primer avance contiene un **prototipo
de consola con un solo carril**. La GUI 2D y los patrones Factory Method, State y
Observer completos se incorporaran de forma progresiva; **todavia no estan
implementados**.

## Que funciona en este avance

- Clase generica `ColaConcurrente<T>` creada desde cero con `std::mutex` y `std::condition_variable`.
- Modelo basico `Vehiculo` con identificador y tipo.
- Clase `Semaforo` con estados rojo y verde y espera bloqueante.
- Tres tareas concurrentes (`std::thread`): generar, controlar semaforo y despachar.
- Registro de la llegada y el paso de 8 vehiculos sin interfaz grafica.
- Prueba de concurrencia con 4 productores, 3 consumidores y 400 elementos.

## Requisitos

- Compilador con soporte **C++17** (GCC, Clang o MSVC moderno).
- Biblioteca estandar de C++ con soporte de hilos.
- **Opcional:** CMake 3.16 o superior para compilar y ejecutar las pruebas con CTest.
- **SFML no se necesita en esta version**: esta prevista para el segundo avance.

## Compilar y ejecutar con CMake

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Ejecutar: `./build/simtrrafic` (Linux) o `build\\Debug\\simtrrafic.exe` (Windows,
si se utiliza el generador de Visual Studio). La ruta depende del generador.

## Alternativa con g++ (sin CMake)

En Linux, WSL o MinGW-w64 con soporte de hilos:

```bash
g++ -std=c++17 -Wall -Wextra -pthread -Iinclude src/main.cpp -o simtrrafic
./simtrrafic

g++ -std=c++17 -Wall -Wextra -pthread -Iinclude tests/test_concurrencia.cpp -o test_concurrencia
./test_concurrencia
```

## Estructura

```text
SimTrrafic/
|-- CMakeLists.txt
|-- README.md
|-- .gitignore
|-- include/
|   |-- Vehiculo.hpp
|   |-- ColaConcurrente.hpp
|   `-- Semaforo.hpp
|-- src/
|   `-- main.cpp
|-- docs/
|   `-- diseno_inicial.md
`-- tests/
    `-- test_concurrencia.cpp
```

## Limites actuales y siguientes entregas

En el primer avance solo se simula **un carril** y se despacha un vehiculo a la
vez. El estado del semaforo se comprueba antes de empezar a cruzar; un vehiculo
que ya comenzo puede terminar su paso cuando la luz cambia. No existen peatones,
colisiones, varias direcciones ni parametros configurables en esta version.

Siguiente iteracion: modelar dos ejes que no tengan luz verde simultaneamente;
separar vehiculos por herencia y Factory Method; migrar el control del semaforo a
State; permitir eventos de Observer; despues visualizar en SFML 3.

El documento `docs/diseno_inicial.md` explica la organizacion prevista del
sistema y distingue las clases implementadas de las que aun faltan.

## Subir a GitHub

Desde la carpeta del proyecto, despues de configurar tu nombre y correo en Git:

```bash
git init
git add .
git commit -m "feat: estructura inicial y prototipo de SimTrrafic"
git branch -M main
git tag v0.1.0
```

Crea un repositorio **vacio** en GitHub y ejecuta los comandos siguientes,
reemplazando `USUARIO` por el propietario real de la cuenta o la organizacion:

```bash
git remote add origin https://github.com/USUARIO/SimTrrafic.git
git push -u origin main
git push origin v0.1.0
```

## Referencias tecnicas

- [Documentacion CMake - CXX_STANDARD](https://cmake.org/cmake/help/latest/prop_tgt/CXX_STANDARD.html)
- [SFML - Getting and Using SFML](https://www.sfml-dev.org/faq/build-use/)
- [SFML - CMake Project Template](https://www.sfml-dev.org/tutorials/3.0/getting-started/cmake/)

