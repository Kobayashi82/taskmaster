# TODO

- en rotacion verificar si es un dispositivo especial y no rotar
- validar 'command' en program
- validar 'programs' en group

### Advertencia de seguridad
Supervisord is running as root and it is searching for its configuration file in default locations (including its current working directory); you probably want to specify a "-c" argument specifying an absolute path to a configuration file for improved security.


2025-08-30 21:25:44,983 CRIT Supervisor is running as root.  Privileges were not dropped because no user is specified in the config file.  If you intend to run as root, you can set user=root in the config file to avoid this message.
