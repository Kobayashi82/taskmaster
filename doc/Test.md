
### Servidor UNIX
  - Socket ya en uso por otro proceso
  - Ruta muy larga (límite del sistema)

### Sintaxis del archivo
- **Casos de prueba**:
  - Caracteres no-ASCII: acentos, emojis
  - Archivo muy grande: 10MB+ de configuración
  - Archivo binario en lugar de texto
  - Encoding inválido: archivo con encoding incorrecto

### Límites del Sistema
- **Casos de prueba**:
  - Muchos procesos: configurar 1000+ programas
  - Nombres muy largos: PATH_MAX length
  - Memory limits: configuración que agote la memoria
  - File descriptor limits: más procesos que ulimit -n
  - Process limits: más procesos que ulimit -u

### Estados del Sistema
- **Casos de prueba**:
  - Disk full: cuando se intenta escribir logs
  - Out of memory: durante la ejecución
  - No space left on device: al crear pidfiles/sockets
  - Clock changes: daylight saving, ntp adjustments

### Casos de Concurrencia
- **Casos de prueba**:
  - Multiple taskmaster instances
  - Señales durante parsing
  - Cambios en filesystem durante lectura
  - Network interfaces down durante HTTP server setup

### Validaciones Cruzadas
- **Casos de prueba**:
  - program referenciado en group que no existe
  - Variables de environment que se referencian mutuamente
  - Dependencias circulares entre programas
  - Configuraciones conflictivas: mismo pidfile para múltiples instancias
