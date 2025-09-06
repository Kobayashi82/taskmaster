# TODO

# Configuración

- Reload config solo busca program: , group: e include. Resto ignorar o mensaje de unknown section.
- Luego si hay un programa con el mismo nombre y hay cambios, detiene y cambia. Si no está en los programas nuevos, lo detiene y elimina y si no ha cambiado, lo deja.
- Con los grupos simplemente borra los que no esten y añade/modifica los que si

- Revisar environment_validate()

## Logging

- En rotacion verificar si es un dispositivo especial y no rotar

## Cliente

- taskmasterctl start myapp → Ejecuta el programa y muestra warning
- taskmasterctl start group myapp → Ejecuta el grupo
- taskmasterctl start program myapp → Ejecuta el program
- También podria añadir taskmasterctl list, taskmasterctl list programs y taskmasterctl list groups

# Notas:

- Eliminar .pid y .sock al cerrar el programa
