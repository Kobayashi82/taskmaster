# TODO

- State Machine
- pipe2(fds, O_CLOEXEC | O_NONBLOCK);

# Buffer de STD

```c
const size_t MAX_BUFFER_SIZE = 1024 * 1024;    // 1MB límite
const size_t MIN_TRIM_SIZE = 100 * 1024;       // 100KB mínimo a eliminar

void trimBuffer() {
    if (buffer.size() <= MAX_BUFFER_SIZE) return;
    
    // Buscar \n después de los primeros 100KB
    auto start_search = buffer.begin() + std::min(MIN_TRIM_SIZE, buffer.size());
    auto it = std::find(start_search, buffer.end(), '\n');
    
    if (it != buffer.end()) {
        // Eliminar desde inicio hasta después del \n
        buffer.erase(buffer.begin(), it + 1);
    } else {
        // No hay \n después de 100KB - forzar corte en MIN_TRIM_SIZE
        buffer.erase(buffer.begin(), buffer.begin() + MIN_TRIM_SIZE);
    }
}
```
# Execution

```c

std::cerr << getpid() << "\n";
std::cerr << TaskMaster.programs[2].process[0].name << "\n";
while (!TaskMaster.silent) ;
std::cerr << TaskMaster.programs[2].process[0].name << "\n";

Log.close();
char **envp = Utils::toArray(TaskMaster.programs[2].process[0].environment);
char **args = Utils::toArray(TaskMaster.programs[2].process[0].arguments);

Utils::environment_print(TaskMaster.programs[2].process[0].environment);
	
execvpe(TaskMaster.programs[2].process[0].command.c_str(), args, envp);

Utils::array_free(envp);
Utils::array_free(args);
```

# Máquina de Estados

## Estados del Proceso

### Estados Principales
- **STOPPED**: Proceso no está ejecutándose y no debería estarlo
- **STARTING**: Proceso se está iniciando (fork/exec realizado, esperando confirmación)
- **RUNNING**: Proceso ejecutándose correctamente y estable
- **BACKOFF**: Proceso falló al arrancar, esperando antes de reintentar
- **STOPPING**: Proceso recibió señal de parada, esperando que termine
- **EXITED**: Proceso terminó de forma esperada (código de salida válido)
- **FATAL**: Proceso ha fallado demasiadas veces, no se reintentará más
- **UNKNOWN**: Estado inicial o error en la detección de estado

## Función update_state()

### STOPPED
**Verificar:**
- ¿Tiene `autostart = true`?
- ¿Se recibió comando manual de START?
- ¿Es momento de reiniciar tras un BACKOFF?

**Acciones:**
- Si debe arrancar: `fork()` + `exec()` → **STARTING**
- Si no: mantener **STOPPED**

### STARTING
**Verificar:**
- ¿El proceso sigue vivo? (`terminated == false`)
- ¿Ha pasado el tiempo `startsecs`?
- ¿El proceso murió durante el arranque? (`terminated == true`)

**Acciones:**
- Si murió: incrementar `restart_count` → **BACKOFF** o **FATAL**
- Si pasó `startsecs` y sigue vivo: → **RUNNING**
- Si no ha pasado el tiempo: mantener **STARTING**

### RUNNING
**Verificar:**
- ¿El proceso sigue vivo? (`terminated == false`)
- ¿Se recibió comando STOP?
- ¿Se recibió comando RESTART?

**Acciones:**
- Si murió inesperadamente: verificar `autorestart` → **BACKOFF**, **EXITED** o **FATAL**
- Si comando STOP: enviar señal → **STOPPING**
- Si comando RESTART: enviar señal → **STOPPING** (con flag de restart)
- Si sigue vivo: mantener **RUNNING**

### BACKOFF
**Verificar:**
- ¿Ha pasado el tiempo de espera? (tiempo incremental: 1s, 2s, 4s...)
- ¿Se alcanzó `startretries`?
- ¿Se recibió comando STOP?

**Acciones:**
- Si pasó el tiempo y quedan intentos: → **STARTING**
- Si se agotaron los intentos: → **FATAL**
- Si comando STOP: → **STOPPED**
- Si no: mantener **BACKOFF**

### STOPPING
**Verificar:**
- ¿El proceso ya terminó? (`terminated == true`)
- ¿Ha pasado `stopwaitsecs`?
- ¿Hay que usar `killasgroup`?

**Acciones:**
- Si terminó: → **STOPPED** o **STARTING** (si era restart)
- Si pasó timeout: `kill()` con SIGKILL → esperar más
- Si timeout final: forzar cleanup → **STOPPED**

### EXITED
**Verificar:**
- ¿El `exit_code` está en `exitcodes` (códigos válidos)?
- ¿Qué dice `autorestart`? (never, unexpected, always)

**Acciones:**
- Si `autorestart = always`: → **STARTING**
- Si `autorestart = never`: → **STOPPED**
- Si `autorestart = unexpected` y exit_code no válido: → **STARTING**
- Sino: → **STOPPED**

### FATAL
**Verificar:**
- ¿Se recibió comando manual START?
- ¿Se cambió la configuración?

**Acciones:**
- Si comando START: reset `restart_count` → **STARTING**
- Sino: mantener **FATAL**

## Variables de Control Necesarias

### Tiempos
- `start_time`: Cuándo se inició el proceso
- `stop_time`: Cuándo terminó el proceso
- `change_time`: Cuándo cambió de estado por última vez
- `current_time`: Tiempo actual para comparaciones

### Contadores
- `restart_count`: Número de reintentos realizados
- `startretries`: Máximo número de reintentos permitidos

### Estados del Proceso
- `terminated`: Marcado por signalfd cuando muere el proceso
- `exit_code`: Código de salida del proceso
- `pid`: PID del proceso actual

### Flags de Control
- `restart_pending`: Flag interno para restart ordenado
- `stop_requested`: Flag interno para stop ordenado

## Flujo de Verificación

```
1. Procesar eventos pendientes
2. Para cada proceso:
   a. Obtener tiempo actual
   b. Switch según estado actual
   c. Verificar condiciones específicas del estado
   d. Ejecutar transición si corresponde
   e. Actualizar timestamps y contadores
3. cleanup de recursos
```

## Consideraciones Especiales

### Grupos de Procesos
- `stopasgroup`: Usar `killpg()` en lugar de `kill()`
- `killasgroup`: Usar `killpg()` para SIGKILL final

### Manejo de Señales
- Orden de señales: `stopsignal` → esperar `stopwaitsecs` → SIGKILL

### Logs y Debugging
- Actualizar `uptime` cuando el proceso esté **RUNNING**
