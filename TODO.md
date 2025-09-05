# TODO

# Config

- [program:*] and [group:*] sections must be unique and not contain colons or brackets

## Program

- Variable de Grupos
- Validar serverurl (obtener de unix o inet clases)
- Globbing en comando

# Group

- Validaciones de grupos
- Warning: "Program 'myapp' has same name as group 'myapp'. Program will take precedence in ambiguous commands"

# Servers

- Validaciones de unix e inet

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
