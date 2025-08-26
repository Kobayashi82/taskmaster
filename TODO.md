# TODO

## 0. Validacion de opciones
- **Validar** opciones

## 1. Variables de entorno
- **Parsing** del formato `KEY1=value1,KEY2=value2`
- **Validación** de nombres de variables (formato válido)
- **Expansión** de variables en valores (`$HOME`, `${VAR}`)
- **Variables especiales** como `%(here)s` (directorio del config)
- **Herencia** entre environment global y de programa
- **Aplicación** al entorno real del proceso

## 2. Sistema de includes
- **Nueva sección** `[includes]` con clave `files`
- **Resolución de paths** relativos al archivo principal

## 4. Validaciones faltantes en campos existentes

### Comando y paths:
- **Verificar** que el comando existe y es ejecutable
- **Validar** paths de `stdout_logfile` y `stderr_logfile`
- **Crear directorios** padre si no existen

## 5. Valores especiales no procesados
- **AUTO** en `stdout_logfile`/`stderr_logfile` (usar childlogdir)
- **NONE** en logs (desactivar logging)

## 6. Process_name templating
- **Plantillas** como `%(program_name)s_%(process_num)02d`
- **Validación** de formato de plantillas
