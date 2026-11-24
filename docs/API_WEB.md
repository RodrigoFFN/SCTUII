# API local de SimTrrafic

## Dirección

`http://127.0.0.1:8080` por defecto. **Solo loopback**. No se necesita internet ni una base de datos.

## Rutas

| Método | Ruta | Descripción |
| --- | --- | --- |
| GET | `/` | Carga el panel de control. |
| GET | `/styles.css` | Estilos incluidos en el proyecto. |
| GET | `/app.js` | Interacción y gráficas. |
| GET | `/api/status` | Instantánea en JSON con cifras, luces y últimos 80 eventos. |
| POST | `/api/start` | Inicia una simulación validando los parámetros. |
| POST | `/api/stop` | Solicita detener la simulación actual. |
| POST | `/api/reset` | Limpia el monitor cuando no hay una ejecución activa. |

`/api/start` utiliza `application/x-www-form-urlencoded` con los campos `vehicles`, `arrival`, `crossing`, `green`, `red` y `mode` (`fixed` o `adaptive`). Si la solicitud es inválida se devuelve HTTP 400; si una operación no puede ejecutarse por el estado actual se devuelve HTTP 409. Un navegador que se conecta desde el propio servidor obtiene la vista y la API desde el mismo origen.

```bash
curl http://127.0.0.1:8080/api/status
curl -X POST http://127.0.0.1:8080/api/start \
  -d 'vehicles=6&arrival=90&crossing=130&green=1000&red=220&mode=adaptive'
curl -X POST http://127.0.0.1:8080/api/stop
curl -X POST http://127.0.0.1:8080/api/reset
```

No hay autenticación porque es una **herramienta para ejecutar en el equipo del estudiante**: no debe exponerse públicamente en la red ni colocarse detrás de un proxy accesible desde internet. Las rutas estáticas se fijan por lista blanca, sin aceptar rutas arbitrarias del usuario. No se permite ejecutar scripts ni comandos desde la interfaz.
