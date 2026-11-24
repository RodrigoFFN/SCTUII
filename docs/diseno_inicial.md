> Documento histórico del tercer avance (`v0.3.0`). Para la versión integrada y la GUI consulte `README.md` y `docs/AVANCE_FINAL.md`.

# Diseño de SimTrrafic

**Curso:** Tecnología de Objetos  
**Etapa:** Tercer avance (`v0.3.0`)

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
| `Carril` | Encapsula la cola de vehículos de un eje y conoce su dirección. |
| `Estadisticas` | Cuenta vehículos generados y atendidos por eje con contadores atómicos. |
| `Simulador` | Crea los cinco hilos, coordina su arranque y parada y devuelve el resumen. |
| `main.cpp` | Crea un `Simulador`, lo ejecuta y captura excepciones. |

## Flujo concurrente

```text
Generador Norte-Sur --> Carril Norte-Sur --> Despachador Norte-Sur --+
                                                                    |
                                                         Interseccion
                                                                    |
Generador Este-Oeste --> Carril Este-Oeste --> Despachador Este-Oeste-+
                                                                    ^
                                                                    |
                                                     Controlador de fases
```

- Cada generador produce seis vehículos (los ids múltiplos de 3 son `Bus`) y cierra su carril.
- Cada despachador bloquea hasta obtener un vehículo y espera la luz verde de su eje.
  Cuando el carril está cerrado y vacío, la extracción devuelve `nullopt` y el despachador termina.
- El controlador alterna los ejes Norte-Sur y Este-Oeste e introduce un intervalo
  con ambas luces rojas entre las fases. El mensaje de cada cambio se imprime bajo
  el mismo `mutex` de salida que el cambio de luz, de modo que el registro en consola
  respeta el orden real de las fases.
- `Interseccion::habilitar` realiza la transición bajo un `mutex` y primero
  pone ambas luces en rojo. `estado()` permite una lectura consistente del par.
- El controlador termina cuando se han atendido todos los vehículos y apaga ambas luces.
- `Estadisticas` usa `std::atomic` con `memory_order_relaxed`: son contadores
  independientes y el resumen se lee después de unir (`join`) todos los hilos.

### Arranque y parada de hilos

`Simulador::ejecutar` crea los cinco hilos (dos generadores, dos despachadores y el
controlador). Todos esperan una barrera (`std::promise<bool>` con `std::shared_future`)
hasta que se hayan creado los cinco. Si la creación de alguno falla, se publica `false`,
los demás hilos salen sin ejecutar su tarea, se hace `join` de todos y se relanza
la excepción original. Con esto no queda ningún hilo sin unir.

**Limitación importante:** se comprueba la autorización de entrada al cruce, pero
no se sigue la ocupación del espacio por cada vehículo. Un vehículo autorizado
puede terminar después del cambio de luz. La exclusión de luces verdes no es
por sí sola una garantía de ausencia de colisiones físicas.

## Arquitectura prevista para siguientes avances

| Componente | Responsabilidad futura |
| --- | --- |
| `Vehiculo` y subclases | Diferenciar vehículos con herencia/polimorfismo (`Auto`, `Bus`). |
| `Peaton` | Esperar turno y cruzar la intersección. |
| Vista SFML | Dibujar el tráfico y responder a controles. |

## Patrones de diseño pendientes

- **Factory Method:** crear los distintos tipos de vehículos (hoy el tipo es una cadena).
- **State:** encapsular estados y transiciones de los semáforos (hoy es un enumerado de dos luces).
- **Observer:** notificar cambios a otras entidades y a la futura vista (hoy se imprime directamente en consola).

**Todavía no se han implementado** estos tres patrones en su versión completa.

## Pruebas

- `tests/test_concurrencia.cpp`: cuatro productores y tres consumidores (400
  elementos sin pérdidas o duplicados); drenaje y cierre de la cola; espera del
  semáforo básico.
- `tests/test_interseccion.cpp`: estado inicial rojo, alternancia de fases,
  bloqueo del eje en rojo, notificación al ponerse verde, cambios concurrentes
  y apagado de ambas luces.

Aún no hay pruebas propias para `Carril`, `Estadisticas` ni `Simulador`; solo se
validan de forma indirecta con la ejecución completa (12 generados, 12 atendidos).

Próximas pruebas: `Carril`, `Estadisticas` y `Simulador`, finalización de hilos
bajo cancelación, carga variable y seguridad de la ocupación física del cruce.
