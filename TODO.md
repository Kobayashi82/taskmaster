# TODO

- Expandir $PROGRAM_NUM (incrementado) al crear clase
- [program:*] and [group:*] sections must be unique and not contain colons or brackets

- en rotacion verificar si es un dispositivo especial y no rotar

 - Warning: "Program 'myapp' has same name as group 'myapp'. Program will take precedence in ambiguous commands"

En comandos ambiguos:

taskmasterctl start myapp → Ejecuta el programa y muestra warning
taskmasterctl start group myapp → Ejecuta el grupo
taskmasterctl start program myapp → Ejecuta el program

También podria añadir taskmasterctl list, taskmasterctl list programs y taskmasterctl list groups

## Not Implemented

- Espacios escapados al final de la linea
- Environment multi-line
- %(ENV_)s format
