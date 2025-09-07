# Casos de Prueba para Taskmaster

## [taskmasterd] - Configuración Global

### General

[x] Key without value  
[x] Key out of section  
[x] Invalid key  

### Boolean Values

[x] boolean  
[x] user  
[x] umask  
[x] directory  
[x] childlogdir  
[x] logfile  
[x] logfile_maxbytes  
[x] logfile_backups  
[x] loglevel  
[x] pidfile  
[x] minfds 
[x] minprocs 
[x] identifier 

## [program:name] - Configuración de Programas

### command
- **Casos de prueba**:
  - Comando absoluto: `command=/bin/ls`
  - Comando en PATH: `command=ls`
  - Con argumentos: `command=ls -la /tmp`
  - Archivo inexistente: `command=/path/que/no/existe`
  - Sin permisos de ejecución: crear archivo sin +x
  - Es un directorio: `command=/tmp`
  - Expansión tilde: `command=~/mi_script.sh`
  - Script con shebang: crear script válido/inválido
  - Comillas: `command="ls -la"`
  - Cadena vacía: `command=`
  - Solo espacios: `command=   `

### process_name
- **Casos de prueba**:
  - Con numprocs=1: `process_name=mi_proceso` (sin ${PROCESS_NUM})
  - Con numprocs>1 y variable: `process_name=${PROGRAM_NAME}_${PROCESS_NUM:02d}`
  - Con numprocs>1 sin variable: `process_name=proceso_fijo` (debería fallar)
  - Variables inválidas: `process_name=${VARIABLE_INEXISTENTE}`
  - Formato inválido: `process_name=${PROCESS_NUM:abc}`
  - Caracteres especiales: `process_name=proceso-con.caracteres_especiales`
  - Muy largo: generar nombre de 1000+ caracteres
  - Cadena vacía: `process_name=`

### numprocs y numprocs_start
- **Casos de prueba**:
  - Valores válidos: `numprocs=1`, `numprocs=10`, `numprocs=10000`
  - Fuera de rango: `numprocs=0`, `numprocs=10001`
  - No numérico: `numprocs=abc`
  - Decimales: `numprocs=5.5`
  - numprocs_start sin numprocs: solo `numprocs_start=5`
  - Combinación: `numprocs=3, numprocs_start=5`
  - Cadena vacía: `numprocs=`

### priority
- **Casos de prueba**:
  - Valores válidos: `priority=0`, `priority=500`, `priority=999`
  - Fuera de rango: `priority=-1`, `priority=1000`
  - No numérico: `priority=high`
  - Decimales: `priority=500.5`
  - Cadena vacía: `priority=`

### autostart y autorestart
- **Casos de prueba**:
  - autostart: valores booleanos válidos/inválidos (igual que nodaemon)
  - autorestart valores válidos: `true`, `false`, `unexpected`, `always`, `never`
  - autorestart inválidos: `sometimes`, `maybe`, `1`, `yes`
  - Case sensitivity: `UNEXPECTED`, `Always`
  - Cadena vacía para ambos

### startsecs y startretries
- **Casos de prueba**:
  - startsecs válidos: `0`, `1800`, `3600`
  - startsecs inválidos: `-1`, `3601`, `abc`
  - startretries válidos: `1`, `50`, `100`
  - startretries inválidos: `0`, `101`, `abc`
  - Decimales para ambos
  - Cadena vacía

### exitcodes
- **Casos de prueba**:
  - Un código: `exitcodes=0`
  - Múltiples códigos: `exitcodes=0,1,2`
  - Con espacios: `exitcodes=0, 1, 2`
  - Códigos negativos: `exitcodes=-1,0,1`
  - Códigos altos: `exitcodes=255,256`
  - No numéricos: `exitcodes=0,abc,2`
  - Sin comas: `exitcodes=0 1 2`
  - Duplicados: `exitcodes=0,0,1`
  - Cadena vacía: `exitcodes=`

### stopsignal
- **Casos de prueba**:
  - Números válidos: `1`, `2`, `9`, `15`
  - Nombres válidos: `HUP`, `INT`, `KILL`, `TERM`, `USR1`, `USR2`
  - Con prefijo SIG: `SIGHUP`, `SIGTERM`
  - Números inválidos: `0`, `100`, `-1`
  - Nombres inválidos: `INVALID`, `STOP`
  - Case sensitivity: `term`, `hup`
  - Cadena vacía: `stopsignal=`

### stopwaitsecs
- **Casos de prueba**:
  - Valores válidos: `1`, `1800`, `3600`
  - Fuera de rango: `0`, `3601`
  - No numérico: `abc`
  - Decimales: `10.5`
  - Cadena vacía: `stopwaitsecs=`

### stdout_logfile y stderr_logfile
- **Casos de prueba**:
  - Valores especiales: `auto`, `none`
  - Case sensitivity: `AUTO`, `None`, `NONE`
  - Rutas válidas: `/tmp/out.log`, `~/logs/out.log`
  - Rutas inválidas: igual que logfile
  - Expansión de variables: `$HOME/${PROGRAM_NAME}_stdout.log`
  - Variables undefined: `${VARIABLE_QUE_NO_EXISTE}.log`
  - Es un directorio: `stdout_logfile=/tmp`
  - Sin permisos: `stdout_logfile=/root/out.log`
  - Directorio padre no existe: `stdout_logfile=/no/existe/out.log`

### Archivos de log *_maxbytes y *_backups
- **Casos similares a logfile_maxbytes y logfile_backups**
- **Casos adicionales**:
  - Combinación maxbytes=0 y backups>0
  - maxbytes>0 y backups=0

### serverurl
- **Casos de prueba**:
  - UNIX socket válido: `unix://~/taskmaster.sock`
  - UNIX socket inválido: `unix://`
  - HTTP válido: `http://localhost:8080`
  - HTTPS válido: `https://127.0.0.1:9001`
  - Puerto fuera de rango: `http://localhost:70000`
  - IP inválida: `http://999.999.999.999:8080`
  - Sin puerto HTTP: `http://localhost`
  - Protocolo inválido: `ftp://localhost:8080`
  - Valor 'auto': `serverurl=auto`
  - Malformado: `http//localhost:8080` (sin :)
  - Cadena vacía: `serverurl=`

## [group:name] - Configuración de Grupos

### programs
- **Casos de prueba**:
  - Programas existentes: `programs=prog1,prog2`
  - Con espacios: `programs=prog1, prog2, prog3`
  - Programa inexistente: `programs=prog1,programa_que_no_existe`
  - Sin comas: `programs=prog1 prog2`
  - Duplicados: `programs=prog1,prog1,prog2`
  - Cadena vacía: `programs=`
  - Solo comas: `programs=,,,`

### priority
- **Igual que priority de programs**

## [include] - Inclusión de Archivos

### files
- **Casos de prueba**:
  - Archivo existente: `files=config.conf`
  - Archivo inexistente: `files=noexiste.conf`
  - Múltiples archivos: `files=conf1.conf,conf2.conf`
  - Globbing válido: `files=conf.d/*.conf`
  - Globbing sin matches: `files=conf.d/*.xyz`
  - Expansión tilde: `files=~/config/*.conf`
  - Patrones complejos: `files=conf.d/[a-z]*.conf`
  - Directorio en lugar de archivo: crear directorio llamado test.conf
  - Sin permisos de lectura: crear archivo sin permisos
  - Recursión infinita: archivo A incluye archivo B que incluye archivo A
  - Cadena vacía: `files=`

## [unix_http_server] - Servidor UNIX

### file
- **Casos similares a pidfile**
- **Casos adicionales**:
  - Socket ya en uso por otro proceso
  - Ruta muy larga (límite del sistema)

### chmod
- **Casos similares a umask**
- **Casos adicionales**:
  - Permisos válidos para sockets: `0700`, `0755`, `0777`

### chown
- **Casos de prueba**:
  - Solo usuario: `chown=1000`
  - Usuario y grupo: `chown=1000:1000`
  - Por nombres: `chown=www-data:www-data`
  - Usuario válido, grupo inválido: `chown=1000:99999`
  - Usuario inválido: `chown=99999:1000`
  - Sin permisos para cambiar owner
  - Formato inválido: `chown=1000:1000:extra`
  - Cadena vacía: `chown=`

### username y password
- **Casos de prueba**:
  - Credenciales válidas: `username=admin`, `password=secret`
  - Password hasheado: `password={SHA}aaf4c61ddcc5e8a2dabede0f3b482cd9aea9434d`
  - Hash inválido: `password={SHA}invalid`
  - Algoritmo inválido: `password={MD5}hash`
  - Username vacío: `username=`
  - Password vacío: `password=`
  - Caracteres especiales: `username=user@domain.com`
  - Username muy largo: 1000+ caracteres
  - Solo uno de los dos configurado

## [inet_http_server] - Servidor HTTP

### port
- **Casos de prueba**:
  - Localhost: `port=localhost:8080`
  - IP específica: `port=127.0.0.1:8080`
  - Todas las interfaces: `port=*:8080`
  - Sin hostname: `port=:8080`
  - Puerto en uso: configurar dos servidores con el mismo puerto
  - Puerto privilegiado sin permisos: `port=localhost:80`
  - IP inválida: `port=999.999.999.999:8080`
  - Puerto fuera de rango: `port=localhost:70000`
  - Hostname inválido: `port=servidor.que.no.existe:8080`
  - Sin puerto: `port=localhost`
  - Formato inválido: `port=localhost::8080`
  - IPv6: `port=[::1]:8080`
  - Cadena vacía: `port=`

## Casos Edge Generales

### Sintaxis del archivo
- **Casos de prueba**:
  - Secciones duplicadas: `[program:test]` aparece dos veces
  - Secciones inválidas: `[invalid_section]`
  - Claves duplicadas en la misma sección
  - Comentarios: líneas con `;` y `#`
  - Líneas vacías y con solo espacios
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
  - System under heavy load
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