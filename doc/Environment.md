
# Variables de Entorno

<br> </br>

## Expansión

### Valores por defecto:

| Valor                       | Descripción                                                    |
|-----------------------------|----------------------------------------------------------------|
| **${VAR:-default}**         | si VAR está vacía o no existe, usa "default"                   |
| **${VAR:=default}**         | como el anterior, pero además asigna "default" a VAR           |
| **${VAR:+algo}**            | si VAR tiene valor, devuelve "algo", sino devuelve vacío       |
|

### Cadena:

| Valor                       | Descripción                                                    |
|-----------------------------|----------------------------------------------------------------|
| **${VAR:2}**                | desde la posición 2 hasta el final                             |
| **${VAR:2:3}**              | desde posición 2, toma 3 caracteres                            |
| **${VAR: -2}**              | los últimos 2 caracteres (espacio antes del -)                 |
| **${VAR:\*upper}**          | convertir a mayúsculas                                         |
| **${VAR:\*lower}**          | convertir a minúsculas                                         |
|

### Númerico:

| Valor                       | Descripción                                                    |
|-----------------------------|----------------------------------------------------------------|
| **${VAR:\*d}**              | entero decimal con signo                                       |
| **${VAR:\*02d}**            | entero con N dígitos (ceros a la izquierda)                    |
| **${VAR:\*x}**              | hexadecimal minúsculas                                         |
| **${VAR:\*X}**              | hexadecimal mayúsculas                                         |
| **${VAR:\*o}**              | octal                                                          |
|

<br> </br>

## Variables de Taskmaster:

| Valor                       | Descripción                                                    |
|-----------------------------|----------------------------------------------------------------|
| **SUPERVISOR_ENABLED**      | flag indicando que el proceso está bajo control de supervisor  |
| **SUPERVISOR_PROCESS_NAME** | nombre del proceso especificado en el archivo de configuración |
| **SUPERVISOR_GROUP_NAME**   | nombre del grupo al que pertenece el proceso                   |
| **SUPERVISOR_SERVER_URL**   | URL del servidor interno (unix or tcp)                         |
|

<br> </br>

## Variables de Configuración

| Valor                       | Descripción                                                    |
|-----------------------------|----------------------------------------------------------------|
| **HOST_NAME**               | nombre del host                                                |
| **GROUP_NAME**              | nombre del grupo                                               |
| **PROGRAM_NAME**            | nombre del programa                                            |
| **NUMPROCS**                | numero de procesos                                             |
| **PROCESS_NUM**             | número de proceso (0, 1, 2...)                                 |
| **HERE**                    | directorio del archivo de configuración actual                 |
|

<br> </br>

## Orden de Creación de Variables

|   #   | Descripción                                                                          |
|-------|--------------------------------------------------------------------------------------|
| **1** | Variables de entorno del shell                                                       |
| **2** | Variables de la sección global                                                       |
| **3** | Variables de Taskmaster                                                              |
| **4** | Variables de la sección program                                                      |
|
