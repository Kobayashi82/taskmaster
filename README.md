<div align="center">

![System & Kernel](https://img.shields.io/badge/System-brown?style=for-the-badge)
![Process Control](https://img.shields.io/badge/Process-Control-blue?style=for-the-badge)
![Job Daemon](https://img.shields.io/badge/Job-Daemon-green?style=for-the-badge)
![C++ Language](https://img.shields.io/badge/Language-C++-red?style=for-the-badge)

*Un daemon de control de trabajos con características similares a supervisor*

</div>

# Taskmaster

## 🎯 Descripción

`Taskmaster` es un proyecto de `42 School` que implementa un daemon completo de control de trabajos con funcionalidades similares a supervisor. Este sistema permite gestionar procesos en segundo plano, mantenerlos vivos, reiniciarlos automáticamente cuando sea necesario y proporcionar un control total sobre su ciclo de vida. El proyecto incluye tanto el daemon principal como un cliente de control con shell interactivo avanzado.

## ✨ Características

### Daemon Principal (taskmasterd)
- **Daemon Real**: Proceso que se ejecuta en segundo plano de forma independiente
- **Control de Procesos**: Supervisión completa del ciclo de vida de procesos
- **Configuración Dinámica**: Recarga de configuración sin parar el daemon (SIGHUP)
- **Sistema de Logging**: Gestión avanzada con rotación automática y soporte syslog
- **Privilege De-escalation**: Capacidad de ejecutarse como usuario específico
- **Múltiples Protocolos**: Conexión vía UNIX sockets e INET sockets
- **Variables Avanzadas**: Soporte para expansión de variables en configuración
- **Gestión de Recursos**: Control de descriptores de archivos y procesos mínimos

### Cliente de Control (taskmasterctl)
- **Shell Interactivo Avanzado**: Con readline personalizado y soporte de modo `vi`
- **Historial Completo**: Con búsqueda y expansión de historial
- **Autocompletado**: Sistema inteligente de completado
- **Attach/Detach**: Conexión y desconexión a procesos supervisados en tiempo real
- **Control Remoto**: Gestión del daemon desde cliente remoto

### Parser de Configuración
- **Sintaxis Supervisor**: Compatible con formato supervisor estándar
- **Parser Propio**: Implementación personalizada para máximo control
- **Variables Dinámicas**: Soporte completo para variables del sistema y personalizadas
- **Modificadores de Variables**: Sintaxis avanzada para manipulación de strings y números

## 🔧 Instalación

```bash
git clone https://github.com/Kobayashi82/taskmaster.git
cd taskmaster
make

# Ejecutables generados en la carpeta bin/:
# taskmasterd     - El daemon principal
# taskmasterctl   - Cliente de control interactivo
```

## 🖥️ Uso del Daemon (taskmasterd)

### Opciones Disponibles

```bash
Usage: taskmasterd: [ OPTION... ]
```

#### Configuración y Ejecución:
- `-c, --configuration=FILENAME`: Archivo de configuración
- `-n, --nodaemon`: Ejecutar en primer plano (no como daemon)
- `-s, --silent`: No mostrar logs en stdout (solo cuando no se ejecuta como daemon)
- `-d, --directory=DIRECTORY`: Directorio de trabajo del daemon
- `-k, --nocleanup`: Prevenir limpieza automática al iniciar

#### Control de Usuario y Permisos:
- `-u, --user=USER`: Ejecutar como usuario específico (o UID numérico)
- `-m, --umask=UMASK`: Máscara umask para subprocesos (por defecto: 022)

#### Sistema de Logging:
- `-l, --logfile=FILENAME`: Archivo de log principal
- `-y, --logfile_maxbytes=BYTES`: Tamaño máximo del archivo de log
- `-z, --logfile_backups=NUM`: Número de backups en rotación
- `-e, --loglevel=LEVEL`: Nivel de log (debug,info,warn,error,critical)
- `-q, --childlogdir=DIRECTORY`: Directorio para logs de procesos hijo
- `-t, --strip_ansi`: Eliminar códigos ANSI de la salida de procesos

#### Control de Instancia:
- `-j, --pidfile=FILENAME`: Archivo PID del daemon
- `-i, --identifier=STR`: Identificador único de esta instancia

#### Requisitos del Sistema:
- `-a, --minfds=NUM`: Número mínimo de descriptores de archivo para iniciar
- `-p, --minprocs=NUM`: Número mínimo de procesos disponibles para iniciar

#### Información:
- `-h, --help`: Mostrar ayuda completa
- `-v, --version`: Mostrar versión del programa

### Uso Básico

```bash
# Ejecutar con configuración básica
sudo ./taskmasterd -c /etc/taskmaster/taskmaster.conf

# Ejecutar en primer plano (desarrollo/debug)
./taskmasterd -c config.conf -n -e debug

# Ejecutar como usuario específico
sudo ./taskmasterd -c config.conf -u user

# Configuración completa de logging
./taskmasterd -c config.conf -l /var/log/taskmaster/daemon.log -y 10MB -z 5 -e info -q /var/log/taskmaster/children/
```

### Gestión del Daemon

```bash
# Verificar estado del daemon
ps aux | grep taskmasterd
cat /var/run/taskmasterd.pid

# Recargar configuración (sin parar procesos no modificados)
sudo kill -HUP $(cat /var/run/taskmasterd.pid)

# Parar daemon gracefully
sudo kill -TERM $(cat /var/run/taskmasterd.pid)

# Ver logs en tiempo real
tail -f /var/log/taskmaster/daemon.log
```

## 🖥️ Uso del Cliente (taskmasterctl)

### Características del Shell Interactivo

- **Readline Personalizado**: Implementación propia
- **Historial Completo**: Navegación, búsqueda y expansión de historial
- **Modo Vi**: Edición de líneas con comandos `vi`
- **Autocompletado**: Completado inteligente de comandos y nombres de programas
- **Sintaxis Coloreada**: Resaltado visual de comandos

### Comandos Disponibles

```bash
# Iniciar cliente interactivo
./taskmasterctl

# Comandos dentro del shell:
taskmaster> status                    # Ver estado de todos los programas
taskmaster> status nginx              # Ver estado de programa específico
taskmaster> start nginx               # Iniciar programa
taskmaster> stop nginx                # Parar programa
taskmaster> restart nginx             # Reiniciar programa
taskmaster> reload                    # Recargar configuración del daemon
taskmaster> shutdown                  # Parar el daemon completamente
taskmaster> attach nginx:0            # Conectar a proceso específico
taskmaster> detach                    # Desconectar de proceso actual
taskmaster> tail nginx stderr         # Ver logs de proceso
taskmaster> clear nginx               # Limpiar logs de proceso
taskmaster> help                      # Mostrar ayuda
taskmaster> quit                      # Salir del cliente
```

### Funciones Avanzadas

```bash
# Attach a proceso específico (modo interactivo)
taskmaster> attach webapp:2
# Ahora estás conectado directamente al proceso
# Ctrl+C para detach y volver al shell de taskmaster

# Ver logs en tiempo real
taskmaster> tail -f nginx stdout
taskmaster> tail -100 webapp stderr

# Gestión granular
taskmaster> start webapp:*            # Iniciar todos los procesos de webapp
taskmaster> stop webapp:0 webapp:1    # Parar procesos específicos
taskmaster> restart all               # Reiniciar todos los programas
```

## ⚙️ Archivo de Configuración

El archivo de configuración utiliza sintaxis similar a supervisor

### Parámetros de Configuración

#### Configuración de Taskmasterd

| Parámetro                   | Descripción                   | Valores                          | Por Defecto              |
|-----------------------------|-------------------------------|----------------------------------|--------------------------|
| **nodaemon**                | Ejecutar en primer plano      | true/false                       | false                    |
| **silent**                  | No mostrar logs en stdout     | true/false                       | false                    |
| **user**                    | Usuario para ejecutar daemon  | string o UID                     | (usuario actual)         |
| **umask**                   | Máscara de permisos           | octal                            | 022                      |
| **directory**               | Directorio de trabajo         | path                             | (directorio actual)      |
| **logfile**                 | Archivo de log del daemon     | path/AUTO/NONE                   | AUTO                     |
| **logfile_maxbytes**        | Tamaño máximo del log         | bytes (KB/MB)                    | 50MB                     |
| **logfile_backups**         | Número de backups de log      | int                              | 10                       |
| **loglevel**                | Nivel de logging              | debug/info/warn/error/critical   | info                     |
| **pidfile**                 | Archivo PID del daemon        | path/AUTO                        | AUTO                     |
| **identifier**              | Identificador de instancia    | string                           | taskmaster               |
| **childlogdir**             | Directorio logs de procesos   | path/AUTO                        | AUTO                     |
| **strip_ansi**              | Eliminar códigos ANSI         | true/false                       | false                    |
| **nocleanup**               | No limpiar al iniciar         | true/false                       | false                    |
| **minfds**                  | Descriptores mínimos          | int                              | 1024                     |
| **minprocs**                | Procesos mínimos disponibles  | int                              | 200                      |
| **environment**             | Variables de entorno globales | KEY=Value                        | (vacío)                  |
|

#### Configuración de Programa

| Parámetro                   | Descripción                   | Valores                          | Por Defecto              |
|-----------------------------|-------------------------------|----------------------------------|--------------------------|
| **command**                 | Comando a ejecutar            | string                           | (requerido)              |
| **numprocs**                | Número de procesos a mantener | int                              | 1                        |
| **process_name**            | Patrón de nombres de proceso  | string                           | $PROGRAM_NAME            |
| **directory**               | Directorio de trabajo         | path                             | /                        |
| **umask**                   | Máscara de permisos           | octal                            | 022                      |
| **user**                    | Usuario para ejecutar         | string                           | (usuario actual)         |
| **autostart**               | Iniciar automáticamente       | bool                             | true                     |
| **autorestart**             | Política de reinicio          | true/false/unexpected            | unexpected               |
| **exitcodes**               | Códigos de salida esperados   | lista                            | 0                        |
| **startretries**            | Intentos de inicio            | int                              | 3                        |
| **starttime**               | Tiempo mínimo ejecutándose    | segundos                         | 1                        |
| **stopsignal**              | Señal para parar gracefully   | TERM/HUP/INT/QUIT/KILL/USR1/USR2 | TERM                     |
| **stoptime**                | Tiempo antes de SIGKILL       | segundos                         | 10                       |
| **priority**                | Prioridad de inicio/parada    | int                              | 999                      |
| **stdout_logfile**          | Archivo de log para stdout    | path/AUTO/NONE                   | AUTO                     |
| **stderr_logfile**          | Archivo de log para stderr    | path/AUTO/NONE                   | AUTO                     |
| **stdout_logfile_maxbytes** | Tamaño máximo stdout          | bytes  (KB/MB)                   | 50MB                     |
| **stderr_logfile_maxbytes** | Tamaño máximo stderr          | bytes  (KB/MB)                   | 50MB                     |
| **stdout_logfile_backups**  | Backups stdout                | int                              | 10                       |
| **stderr_logfile_backups**  | Backups stderr                | int                              | 10                       |
| **environment**             | Variables de entorno globales | KEY=Value                        | (vacío)                  |
|

#### Configuración de Grupo

| Parámetro                   | Descripción                   | Valores                          | Por Defecto              |
|-----------------------------|-------------------------------|----------------------------------|--------------------------|
| **programs**                | Lista de programas del grupo  | lista                            | (requerido)              |
| **priority**                | Prioridad de inicio del grupo | int                              | 999                      |
|

#### Configuración del servidor UNIX

| Parámetro                   | Descripción                   | Valores                          | Por Defecto              |
|-----------------------------|-------------------------------|----------------------------------|--------------------------|
| **file**                    | Ruta del socket UNIX          | path                             | /var/run/taskmaster.sock |
| **chmod**                   | Permisos del socket           | octal                            | 0700                     |
| **chown**                   | Propietario del socket        | user:group                       | root:root                |
|

#### Configuración del servidor INET

| Parámetro                   | Descripción                   | Valores                          | Por Defecto              |
|-----------------------------|-------------------------------|----------------------------------|--------------------------|
| **port**                    | Host/IP y puerto de escucha   | host:port o *:port               | *:9001                   |
| **username**                | Usuario para autenticación    | string                           | (sin auth)               |
| **password**                | Contraseña para autenticación | string                           | (sin auth)               |
|

### Ejemplo de archivo de configuración

```yaml
# Configuración global del servidor
[taskmasterd]
nodaemon=false
silent=false
user=taskmaster
umask=077
directory=$HERE
logfile=/var/log/taskmaster/daemon.log
logfile_maxbytes=50MB
logfile_backups=10
loglevel=info
pidfile=/var/run/taskmasterd.pid
identifier=main
childlogdir=/var/log/taskmaster/children
strip_ansi=true
nocleanup=false
minfds=512
minprocs=100
environment=KEY1=VALUE1, KEY2=VALUE2

# Configuración de conectividad
[unix_http_server]
file=/var/run/taskmaster.sock
chmod=0700
chown=taskmaster:taskmaster

[inet_http_server]
port=127.0.0.1:9001
username=admin
password=secret

# Definición de programas
[program:nginx]
command=/usr/local/bin/nginx -c /etc/nginx/test.conf
numprocs=1
directory=/tmp
umask=022
autostart=true
autorestart=unexpected
exitcodes=0,2
startretries=3
starttime=5
stopsignal=TERM
stoptime=10
stdout_logfile=/var/log/nginx/stdout.log
stderr_logfile=/var/log/nginx/stderr.log
environment=STARTED_BY="taskmaster",ANSWER="42",PATH="/usr/bin:${PATH}"
user=nginx
priority=999

[program:webapp]
command=/usr/local/bin/webapp --config=${HERE}/webapp.conf
numprocs=4
process_name=$PROGRAM_NAME_${PROCESS_NUM:02d}
directory=/opt/webapp
umask=077
autostart=true
autorestart=true
exitcodes=0
startretries=5
starttime=3
stopsignal=TERM
stoptime=15
stdout_logfile=/var/log/webapp/%(process_name)s.out
stderr_logfile=/var/log/webapp/%(process_name)s.err
environment=WEBAPP_ENV="production", WEBAPP_PORT="800${PROCESS_NUM:d}", DB_HOST="${DB_HOST:-localhost}", WORKER_ID="${PROCESS_NUM:d}"
user=webapp

# Definición de grupos
[group:mygroup]
programs=nginx,webapp
priority=999
```

## 🔤 Variables

El sistema de variables de Taskmaster permite una configuración dinámica y flexible mediante expansión de variables con sintaxis avanzada. Soporta valores por defecto, manipulación de cadenas, formateo numérico y variables de configuración

#### Valores por Defecto

| Sintaxis                    | Descripción                                                     | Ejemplo             |
|-----------------------------|-----------------------------------------------------------------|---------------------|
| **${VAR:-default}**         | Si `VAR` está vacía o no existe, usa `default`                  | ${PORT:-8080}       |
| **${VAR:+value}**           | Si `VAR` tiene valor, devuelve `value`, sino devuelve vacío     | ${DEBUG:+--verbose} |
|

#### Manipulación de Cadenas

| Sintaxis                    | Descripción                                                     | Ejemplo             |
|-----------------------------|-----------------------------------------------------------------|---------------------|
| **${VAR:2}**                | Desde posición 2 hasta el final                                 | ${PATH:2}           |
| **${VAR:2:3}**              | Desde posición 2, toma 3 caracteres                             | ${USER:0:3}         |
| **${VAR: -2}**              | Últimos 2 caracteres (espacio antes del `-`)                    | ${HOST: -2}         |
| **${VAR:\*upper}**          | Convertir a mayúsculas                                          | ${USER:*upper}      |
| **${VAR:\*lower}**          | Convertir a minúsculas                                          | ${USER:*lower}      |
|

#### Formateo Numérico

| Sintaxis                    | Descripción                                                     | Ejemplo             |
|-----------------------------|-----------------------------------------------------------------|---------------------|
| **${VAR:\*d}**              | Entero decimal con signo                                        | ${PROCESS_NUM:*d}   |
| **${VAR:\*02d}**            | Entero con ceros a la izquierda                                 | ${PROCESS_NUM:*02d} |
| **${VAR:\*x}**              | Hexadecimal minúsculas                                          | ${PORT:*x}          |
| **${VAR:\*X}**              | Hexadecimal mayúsculas                                          | ${PORT:*X}          |
| **${VAR:\*#x}**             | Hexadecimal minúsculas (precedido de 0x)                        | ${PORT:*#x}         |
| **${VAR:\*#X}**             | Hexadecimal mayúsculas (precedido de 0X)                        | ${PORT:*#X}         |
| **${VAR:\*o}**              | Octal                                                           | ${UMASK:*o}         |    
|

#### Variables de Taskmaster

| Sintaxis                    | Descripción                                                                           |
|-----------------------------|---------------------------------------------------------------------------------------|
| **SUPERVISOR_ENABLED**      | Flag indicando que el proceso está bajo control de Taskmaster                         |
| **SUPERVISOR_PROCESS_NAME** | Nombre del proceso especificado en el archivo de configuración                        |
| **SUPERVISOR_GROUP_NAME**   | Nombre del grupo al que pertenece el proceso                                          |
| **SUPERVISOR_SERVER_URL**   | URL del servidor interno                                                              |
|

#### Variables de Configuración

| Variable                    | Descripción                                                                           |
|-----------------------------|---------------------------------------------------------------------------------------|
| **HOST_NAME**               | Nombre del host del sistema                                                           |
| **GROUP_NAME**              | Nombre del grupo actual                                                               |
| **PROGRAM_NAME**            | Nombre del programa                                                                   |
| **NUMPROCS**                | Número total de procesos                                                              |
| **PROCESS_NUM**             | Número de proceso (0, 1, 2...)                                                        |
| **HERE**                    | Directorio del archivo de configuración                                               |
|

## 🧪 Testing

### Pruebas Básicas del Daemon

```bash
# Test de inicio básico
./taskmasterd -c test.conf -n -e debug

# Test de configuración inválida
./taskmasterd -c invalid.conf -n

# Test de privilege de-escalation
sudo ./taskmasterd -c test.conf -u nobody -n

# Test de archivos de lock
./taskmasterd -c test.conf
./taskmasterd -c test.conf  # Debería fallar
```

### Pruebas de Control de Procesos

```bash
# Crear configuración de prueba con programa simple
cat > test.conf << EOF
[program:test]
command=/bin/sleep 30
numprocs=2
autostart=true
autorestart=true
EOF

# Ejecutar daemon y cliente
./taskmasterd -c test.conf -n
./taskmasterctl

# En el cliente:
taskmaster> status
taskmaster> stop test:0
taskmaster> start test:0
taskmaster> restart test
```

### Pruebas de Variables

```bash
# Configuración con variables complejas
cat > vars.conf << EOF
[program:webapp]
command=/usr/bin/webapp --port=800${PROCESS_NUM:*02d} --name=${PROGRAM_NAME}_${PROCESS_NUM}
environment=LOG_LEVEL="${LOG_LEVEL:-info}", WORKER_NAME="${USER:*upper}_${PROCESS_NUM:*02d}", CONFIG_PATH="${HERE}/config"
numprocs=3
EOF

# Test de expansión de variables
LOG_LEVEL=debug ./taskmasterd -c vars.conf -n
```

### Pruebas de Logging y Rotación

```bash
# Test de rotación de logs
./taskmasterd -c test.conf -l /tmp/daemon.log -y 1KB -z 3 -n

# Generar logs hasta rotación
./taskmasterctl << EOF
status
tail test:0 stdout
tail test:1 stderr
EOF
```

### Pruebas de Attach/Detach

```bash
# Configuración con programa interactivo
cat > interactive.conf << EOF
[program:shell]
command=/bin/bash
numprocs=1
autostart=true
stdout_logfile=NONE
stderr_logfile=NONE
EOF

# Test de attach
./taskmasterd -c interactive.conf -n
./taskmasterctl

# En el cliente:
taskmaster> attach shell:0
# Ahora estás en bash interactivo
# Ctrl+C para detach
```

### Pruebas de Conexión Remota

```bash
# Configuración con servidor inet
cat > inet.conf << EOF
[inet_http_server]
port=127.0.0.1:9001

[program:test]
command=/bin/echo "Hello World"
EOF

# Conectar desde cliente remoto
./taskmasterctl -s http://127.0.0.1:9001
```

## 📝 Ejemplos de Log

```bash
2024-08-28 10:30:15,123 INFO     taskmasterd started with pid 12345
2024-08-28 10:30:15,124 INFO     spawned: 'nginx' with pid 12346
2024-08-28 10:30:15,125 INFO     spawned: 'webapp_00' with pid 12347
2024-08-28 10:30:15,126 INFO     spawned: 'webapp_01' with pid 12348
2024-08-28 10:30:20,130 INFO     success: nginx entered RUNNING state, process has stayed up for > than 5 seconds (startsecs)
2024-08-28 10:30:20,131 INFO     success: webapp_00 entered RUNNING state, process has stayed up for > than 3 seconds (startsecs)
2024-08-28 10:30:25,135 WARN     webapp_01 process terminated unexpectedly (exit code 1)
2024-08-28 10:30:25,136 INFO     spawned: 'webapp_01' with pid 12350
2024-08-28 10:30:28,140 INFO     success: webapp_01 entered RUNNING state, process has stayed up for > than 3 seconds (startsecs)
2024-08-28 10:31:15,200 INFO     received SIGHUP indicating configuration reload
2024-08-28 10:31:15,201 INFO     configuration reloaded successfully
2024-08-28 10:31:15,202 INFO     stopped: nginx (exit status 0)
2024-08-28 10:31:15,203 INFO     spawned: 'nginx' with pid 12355
2024-08-28 10:31:20,210 INFO     success: nginx entered RUNNING state
```

## 🏗️ Arquitectura Técnica

### Estructura del Daemon
- **Daemonización**: Fork doble para independencia completa del terminal
- **Control de Instancia**: Archivos PID y lock para evitar múltiples instancias
- **Gestión de Señales**: Manejo completo de SIGHUP, SIGTERM, SIGINT, SIGCHLD
- **Privilege De-Escalation**: Cambio seguro de usuario después del inicio

### Sistema de Procesos
- **Supervisión**: Monitoreo continuo del estado de procesos hijo
- **Reinicio Inteligente**: Políticas configurables de reinicio automático
- **Timeout Control**: Gestión de tiempos de inicio y parada
- **Control de Recursos**: Verificación de descriptores y procesos disponibles

### Sistema de Comunicación
- **UNIX Sockets**: Comunicación local de alta velocidad
- **INET Sockets**: Acceso remoto con autenticación
- **Protocolo Propio**: Mensajes estructurados para comunicación cliente-daemon
- **Attach/Detach**: Multiplexación de I/O para acceso directo a procesos

### Parser de Configuración
- **Sintaxis Supervisor**: Compatibilidad con configuraciones existentes
- **Variables Dinámicas**: Expansión avanzada con modificadores
- **Validación**: Verificación completa de sintaxis y semántica
- **Recarga en Caliente**: Actualización sin interrumpir servicios estables

### Sistema de Logging
- **Rotación Automática**: Basada en tamaño y número de archivos
- **Múltiples Niveles**: DEBUG, INFO, WARN, ERROR, CRITICAL
- **Syslog Integration**: Envío opcional a sistema `syslog`
- **Logs por Proceso**: Separación de `stdout`/`stderr` por proceso

## 📄 Licencia

Este proyecto está licenciado bajo la WTFPL – [Do What the Fuck You Want to Public License](http://www.wtfpl.net/about/).

---

<div align="center">

**🌍 Desarrollado como parte del curriculum de 42 School 🌍**

*"Because processes need a reliable babysitter"*

</div>
