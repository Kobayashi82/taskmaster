# TODO

## Program

- Variable de configuración de Grupos
- Globbing en comando

# Servers

- Unix: Separar chown_str en chown_user y chown_group
- Inet: Separar url de puerto

## Logging

- En rotacion verificar si es un dispositivo especial y no rotar

## Variables

- ${VAR-default}
- ${VAR+default}
- ${VAR: -offset:len}
- ${VAR:^}
- ${VAR:^^}
- ${VAR:,}
- ${VAR:,,}
- ${VAR:~}
- ${VAR:~~}
- ${#VAR}
- ${VAR:d}
- ${VAR:02d}
- ${VAR:x}
- ${VAR:X}
- ${VAR:#x}
- ${VAR:#X}
- ${VAR:o}
- +=

## Cliente

- taskmasterctl start myapp → Ejecuta el programa y muestra warning
- taskmasterctl start group myapp → Ejecuta el grupo
- taskmasterctl start program myapp → Ejecuta el program
- También podria añadir taskmasterctl list, taskmasterctl list programs y taskmasterctl list groups

## Not Implemented (for compability with supervisor config files)

- Parser de archivo con soporte de tokens (multi-line)
- Expansion de variables al estilo python

# Notas:

- Eliminar .pid y .sock al cerrar el programa
