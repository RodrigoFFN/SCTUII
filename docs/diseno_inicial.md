# Diseño de SimTrrafic

**Curso:** Tecnología de Objetos  
**Etapa:** Segundo avance (`v0.2.0`)

## Objetivo

Simular una intersección urbana con vehículos, carriles y semáforos coordinados.
El sistema final se proyecta con peatones y visualización 2D (SFML). Actualmente
es una simulación funcional por consola.

## Componentes implementados

| Componente | Responsabilidad actual |
| --- | --- |
| `Vehiculo` | Representa un vehículo con identificador y tipo. |
| `ColaConcurrente<T>` | Mantiene una cola protegida y notifica a los consumidores. |
| `Semaforo` | Guarda una luz roja o verde; permite esperar verde sin espera activa. |
| `Interseccion` | Administra dos semáforos y evita verdes simultáneos. |
| `main.cpp` | Crea productores, consumidores y controlador; recoge resultados. |

## Flujo concurrente

```text
Generador Norte-Sur --> Cola Norte-Sur --> Despachador Norte-Sur --+
                                                                 |
                                                      Interseccion
                                                                 |
Generador Este-Oeste --> Cola Este-Oeste --> Despachador Este-Oeste-+
                                                                 ^
                                                                 |
                                                  Controlador de fases
```

- Cada generador produce seis vehículos y cierra su cola.
- Cada despachador bloquea hasta obtener un vehículo y espera la luz verde.
- El controlador alterna los ejes Norte-Sur y Este-Oeste e introduce un intervalo
  con ambas luces rojas entre las fases.
- `Interseccion::habilitar` realiza la transición bajo un `mutex` y primero
  pone ambas luces en rojo. `estado()` permite una lectura consistente del par.
- Se cuentan los vehículos atendidos para comprobar el fin de la simulación.

**Limitación importante:** se comprueba la autorización de entrada al cruce, pero
no se sigue la ocupación del espacio por cada vehículo. Un vehículo autorizado
puede terminar después del cambio de luz. La exclusión de luces verdes no es
por sí sola una garantía de ausencia de colisiones físicas.

## Arquitectura prevista para siguientes avances

| Componente | Responsabilidad futura |
| --- | --- |
| `Simulador` | Coordinar la inicialización y parada de hilos. |
| `Carril` | Encapsular la cola de vehículos por dirección. |
| `Vehiculo` y subclases | Diferenciar vehículos con herencia/polimorfismo. |
| `Peaton` | Esperar turno y cruzar la intersección. |
| Vista SFML | Dibujar el tráfico y responder a controles. |

## Patrones de diseño pendientes

- **Factory Method:** crear los distintos tipos de vehículos.
- **State:** encapsular estados y transiciones de los semáforos.
- **Observer:** notificar cambios a otras entidades y a la futura vista.

**Todavía no se han implementado** estos tres patrones en su versión completa.

## Pruebas

- `tests/test_concurrencia.cpp`: cuatro productores y tres consumidores (400
  elementos sin pérdidas o duplicados); drenaje y cierre de la cola; espera del
  semáforo básico.
- `tests/test_interseccion.cpp`: estado inicial rojo, alternancia de fases,
  bloqueo del eje en rojo, notificación al ponerse verde, cambios concurrentes
  y apagado de ambas luces.

Próximas pruebas: finalización de hilos bajo cancelación, carga variable y
seguridad de la ocupación física del cruce.
