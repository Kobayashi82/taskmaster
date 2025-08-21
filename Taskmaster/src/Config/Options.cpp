/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Options.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/12 12:15:32 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/21 20:23:23 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Options.hpp"

	#include <algorithm>														// std::transform()
	#include <iostream>															// std::cerr()
	#include <cstring>															// std::strlen()
	#include <getopt.h>															// getopt_long()

#pragma endregion

#pragma region "Variables"

	bool		Options::disabledEncryption	= false;							// Disable encrypted communication 
	bool		Options::disabledShell		= false;							// Disable remote shell
	uint16_t	Options::maxClients			= 3;								// Maximum number of clients connected simultaneously
	uint16_t	Options::portNumber			= 4242;								// Port to listen for incoming connections
	uint16_t	Options::timeout			= 3600;								// Timeout in seconds for inactive connections
	std::string	Options::logFile	= "/var/log/matt_daemon/matt_daemon.log";	// Path to use for logging
	uint8_t		Options::logLevel			= INFO;								// Level of logging
	bool		Options::logNew				= false;							// Create a new log file on start
	uint8_t		Options::logMax				= 5;								// Maximum number of log files to keep when rotating
	size_t		Options::logSize			= 10 *1024 * 1024;					// Minimum log size before rotation
	std::string	Options::shellPath			= "";								// Path of the shell to execute

	int			Options::signum				= 0;								// Signal value when a signal is intercepted
	int			Options::lockfd				= -1;								// FD for daemon lock
	int			Options::sockfd				= -1;								// FD for the socket

	std::string	Options::_fullName			= "MattDaemon";						// Name and path used to execute the program (same as argv[0])

#pragma endregion

#pragma region "Strtoul"

	template<typename T>
	int Options::ft_strtoul(char **argv, const char *optarg, T *value, unsigned long max_value, bool allow_zero) {
		try {
			size_t			idx;
			unsigned long	tmp = std::stoul(optarg, &idx, 0);

			if (idx != std::strlen(optarg)) {
				std::cerr << argv[0] << ": invalid value (`" << optarg << "' near `" << (optarg + idx) << "')\n";
				return (1);
			}

			if (!tmp && !allow_zero) {
				std::cerr << argv[0] << ": option value too small: " << optarg << "\n";
				return (1);
			}

			if (max_value && tmp > max_value) {
				std::cerr << argv[0] << ": option value too big: " << optarg << "\n";
				return (1);
			}

			*value = static_cast<T>(tmp);
			return (0);
		} catch (const std::exception &) {
			std::cerr << argv[0] << ": invalid number: " << optarg << "\n";
			return (1);
		}
	}

#pragma endregion

#pragma region "Messages"

	#pragma region "Help"

		int Options::help() {
			std::cerr << "Usage: " << NAME << " [ OPTION... ] \n";
			std::cerr << "\n";
			std::cerr << " Options:\n";
			std::cerr << "\n";
			std::cerr << "  -c,  --configuration=FILENAME   Configuration file path\n";
			std::cerr << "  -n,  --nodaemon                 Run in the foreground\n";
			std::cerr << "  -s,  --silent                   No logs to stdout\n";
			std::cerr << "  -u,  --user=USER                Run taskmasterd as this user (or numeric uid)\n";
			std::cerr << "  -m,  --umask=UMASK              Use this umask for daemon subprocess (default is 022)\n";
			std::cerr << "  -d,  --directory=DIRECTORY      Directory to chdir to when daemonized\n";
			std::cerr << "  -l,  --logfile=FILENAME         Use FILENAME as logfile path\n";
			std::cerr << "  -y,  --logfile_maxbytes=BYTES   Use BYTES to limit the max size of logfile\n";
			std::cerr << "  -z,  --logfile_backups=NUM      Number of backups to keep when max bytes reached\n";
			std::cerr << "  -e,  --loglevel=LEVEL           Use LEVEL as log level (debug,info,warn,error,critical)\n";
			std::cerr << "  -j,  --pidfile=FILENAME         Write a pid file for the daemon process to FILENAME\n";
			std::cerr << "  -i,  --identifier=STR           Identifier used for this instance of taskmasterd\n";
			std::cerr << "  -q,  --childlogdir=DIRECTORY    The log directory for child process logs\n";
			std::cerr << "  -k,  --nocleanup                Prevent the process from performing cleanup at startup\n";
			std::cerr << "  -a,  --minfds=NUM               The minimum number of file descriptors for start success\n";
			std::cerr << "  -t,  --strip_ansi               Strip ansi escape codes from process output\n";
			std::cerr << "  -p,  --minprocs=NUM             The minimum number of processes available for start success\n";
			std::cerr << "\n";
			std::cerr << "  -h,  --help                     Display this help message\n";
  			std::cerr << "  -v,  --version                  Show program version\n";
			std::cerr << "\n";
			std::cerr << "Report bugs to <kobayashi82@outlook.com>\n";

			return (1);
		}

	#pragma endregion

	#pragma region "Version"

		int Options::version() {
			std::cerr << NAME << " 1.0 part of Taskmaster\n";
			std::cerr << "Copyright (C) 2025 Kobayashi Corp ⓒ.\n";
			std::cerr << "\n";
			std::cerr << "License WTFPL: DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE.\n";
			std::cerr << "This is free software: you are free to change and redistribute it.\n";
			std::cerr << "There is NO WARRANTY, to the extent permitted by law.\n";
			std::cerr << "\n";
			std::cerr << "Written by Kobayashi82 (vzurera-).\n";

			return (1);
		}

	#pragma endregion

	#pragma region "Invalid"

		int Options::invalid() {
			std::cerr << "Try '" << _fullName << " --help' for more information.\n";
			return (2);
		}

	#pragma endregion

	#pragma region "Log Level"

		int Options::validate_loglevel(const std::string& level) {
			std::string l = level;
			std::transform(l.begin(), l.end(), l.begin(), ::tolower);

			if		(l == "debug")		logLevel = DEBUG;
			else if	(l == "info")		logLevel = INFO;
			else if	(l == "warning")	logLevel = WARNING;
			else if	(l == "warn")		logLevel = WARNING;
			else if	(l == "error")		logLevel = ERROR;
			else if	(l == "critical")	logLevel = CRITICAL;
			else { std::cerr << "Invalid log level. Valid values are: DEBUG, INFO, WARNING, CRITICAL\n"; return (1); }

			return (0);
		}

	#pragma endregion

	int Options::validate_path(const std::string& path) {
		(void) path;
		// Convertir ~
		// 
		return (0);
	}


#pragma endregion

#pragma region "Parse"

	int Options::parse(int argc, char **argv) {
		_fullName = argv[0];

		struct option long_options[] = {
			{"configuration",		required_argument,	0, 'c'},	// [-c, --configuration=FILENAME]				- Ruta por defecto: /etc/taskmasterd.conf
			{"nodaemon",			no_argument,		0, 'n'},	// [-n, --nodaemon]								- Muestra log y sale con sañales
			{"silent",				no_argument,		0, 's'},	// [-s, --silent]								- Oculta logs en el stdout desde debug a warning (solo funciona con -n)
			{"user",				required_argument,	0, 'u'},	// [-u, --user=USER]							- Cambia a este usuario después del inicio (privilege de-escalation) (requiere root)
			{"umask",				required_argument,	0, 'm'},	// [-m, --umask=UMASK]							- Establece la máscara de permisos para archivos creados								(por defecto: 022)
			{"directory",			required_argument,	0, 'd'},	// [-d, --directory=DIRECTORY]					- Establece el directorio de trabajo inicial											(por defecto: )
			{"logfile",				required_argument,	0, 'l'},	// [-l, --logfile=FILENAME]						- Archivo donde escribir los logs del daemon											(por defecto: )
			{"logfile_maxbytes",	required_argument,	0, 'y'},	// [-y, --logfile_maxbytes=BYTES]				- Tamaño máximo del archivo de log antes de rotarlo										(por defecto: 10MB)
			{"logfile_backups",		required_argument,	0, 'z'},	// [-z, --logfile_backups=NUM]					- Número de archivos de backup a mantener durante rotación								(por defecto: 5)
			{"loglevel",			required_argument,	0, 'e'},	// [-e, --loglevel=LEVEL]						- Nivel de logging: debug, info, warning, error, critical								(por defecto: info)
			{"pidfile",				required_argument,	0, 'j'},	// [-j, --pidfile=FILENAME]						- Archivo donde escribir el PID del proceso taskmaster
			{"identifier",			required_argument,	0, 'i'},	// [-i, --identifier=STR]						- Identificador único para esta instancia de taskmaster (Se usa en logs y comunicación)
			{"childlogdir",			required_argument,	0, 'q'},	// [-q, --childlogdir=DIRECTORY]				- Directorio donde los procesos hijos escriben sus logs por defecto
			{"nocleanup",			no_argument,		0, 'k'},	// [-k, --nocleanup]							- No limpia archivos temporales al salir
			{"minfds",				required_argument,	0, 'a'},	// [-a, --minfds=NUM]							- Número mínimo de file descriptors requeridos											(por defecto: 1024)
			{"strip_ansi",			no_argument,		0, 't'},	// [-t, --strip_ansi]							- Número mínimo de procesos disponibles en el sistema									(por defecto: 200)
			{"minprocs",			required_argument,	0, 'p'},	// [-p, --minprocs=NUM]							- Elimina secuencias de escape ANSI de los logs de procesos hijos
			{"help",				no_argument,		0, 'h'},	// [-h, --help]
			{"version",				no_argument,		0, 'V'},	// [-V, --version]
			{0, 0, 0, 0}
		};

		int opt;
		while ((opt = getopt_long(argc, argv, "c:nsu:m:d:l:y:z:e:j:i:q:ka:tp:hv", long_options, NULL)) != -1) {
			switch (opt) {
				case 'k':	disabledEncryption = true;																	break;
				case 's':	disabledShell = true;																		break;
				case 'c':	if (ft_strtoul(argv, optarg, &maxClients, 1024 , true))				return (2);				break;
				case 'x':	shellPath = std::string(optarg);															break;

				case '?':	if (std::string(argv[optind - 1]) == "-?")							return (help());		return (invalid());
				case 'h':																		return (help());
				case 'v':																		return (version());
			}
		}

		if (optind < argc) {
			std::cerr << NAME << ": invalid argument: " << argv[optind] << "\n";
			invalid(); return (2);
		}

		return (0);
	}

#pragma endregion
