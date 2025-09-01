# TODO

- expandir variables en include_parse_files()
- en rotacion verificar si es un dispositivo especial y no rotar
- validar rutas al crear clases
- validar 'command' en program
- validar 'programs' en group

- Load file
- Validar keys
- Merge options
- Crear programs
- Crear grupos
- Crear servidores


Programas
---
- Crear variables de entorno 
- Crear variables de taskmaster
- Expandir $PROGRAM_NUMS y $PROGRAM_NUM (incrementado)
- Crear clase completa 

Grupos
---
- Crear grupo


### Advertencia de seguridad
Supervisord is running as root and it is searching for its configuration file in default locations (including its current working directory); you probably want to specify a "-c" argument specifying an absolute path to a configuration file for improved security.
2025-08-30 21:25:44,983 CRIT Supervisor is running as root.  Privileges were not dropped because no user is specified in the config file.  If you intend to run as root, you can set user=root in the config file to avoid this message.

## Not Implemented

- Espacios escapados al final de la linea
- Environment multi-line
- %(ENV_)s format
