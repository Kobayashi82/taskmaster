# TODO

- [program:*] and [group:*] sections must be unique and not contain colons or brackets
- Warning: "Program 'myapp' has same name as group 'myapp'. Program will take precedence in ambiguous commands"
- Validar serverurl (obtener de unix o inet clases)
- Variable de Grupos
- En rotacion verificar si es un dispositivo especial y no rotar
- Variables de entorno: añadir modificadores que faltan (Bash, Python y +=)
- Trim con control de linea final escapada
- Validaciones de grupos, unix e inet

En comandos ambiguos:

taskmasterctl start myapp → Ejecuta el programa y muestra warning
taskmasterctl start group myapp → Ejecuta el grupo
taskmasterctl start program myapp → Ejecuta el program

También podria añadir taskmasterctl list, taskmasterctl list programs y taskmasterctl list groups

## Not Implemented

- Parser de archivo con soporte de tokens (multi-line)
