# Diseño inicial de SimTrrafic

**Curso:** Tecnología de Objetos  
**Etapa:** Primer avance (`v0.1.0`)

## Descripción

El proyecto representará una intersección urbana mediante vehículos, carriles,
peatones y semáforos. La aplicación final utilizará una interfaz gráfica 2D con
SFML para mostrar la circulación y permitirá modificar algunas condiciones de
la simulación.

## Qué está implementado

- `Vehiculo`: estructura que almacena un identificador y el tipo de vehículo.
- `ColaConcurrente<T>`: cola genérica protegida con mutex y variable de condición.
- `Semaforo`: controla los estados rojo y verde, y permite esperar una luz verde.
- `main.cpp`: crea un generador, un controlador y un despachador en tres hilos;
  simula el paso de ocho vehículos por **un único carril**.

## Arquitectura propuesta para versiones posteriores

| Componente | Responsabilidad prevista |
| --- | --- |
| `Simulador` | Coordinar la actualización y el ciclo de vida de la simulación. |
| `Interseccion` | Reunir carriles, semáforos y cruces peatonales. |
| `Carril` | Gestionar una cola de vehículos en una dirección. |
| `Vehiculo` y subclases | Representar distintos tipos de vehículos con herencia y polimorfismo. |
| `Peaton` | Simular la espera y el cruce de peatones. |
| `Semaforo` | Definir estados y reglas de transición. |
| Vista SFML | Dibujar el escenario y responder a los controles del usuario. |

Las clases de esta tabla **son propuestas**: en el primer avance aún no están
programadas, salvo las versiones sencillas de `Vehiculo` y `Semaforo`.

## Patrones de diseño previstos

- **Factory Method:** creación de distintos tipos de vehículos.
- **State:** estados y transiciones del semáforo.
- **Observer:** notificación de cambios de semáforo a entidades interesadas.

Estos patrones se implementarán y comprobarán en entregas posteriores; en esta
versión todavía no se presentan como funcionalidades terminadas.

## Pruebas previstas

La prueba actual verifica el uso simultáneo de una cola con cuatro productores
y tres consumidores, además de la espera de un semáforo. Las siguientes pruebas
deberán comprobar que dos sentidos conflictivos no reciban paso al mismo tiempo,
que los hilos terminen limpiamente y que no se pierdan vehículos durante la
simulación.
