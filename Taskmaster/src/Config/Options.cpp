/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Options.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/12 12:15:32 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/20 22:06:36 by vzurera-         ###   ########.fr       */
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
			std::cerr << "  -c,  --configuration=FILENAME   Configuration file path (searches if not given)\n";
			std::cerr << "  -n,  --nodaemon                 Run in the foreground (same as 'nodaemon=true' in config file)\n";
			std::cerr << "  -s,  --silent                   No logs to stdout (maps to 'silent=true' in config file)\n";
			std::cerr << "  -u,  --user=USER                run supervisord as this user (or numeric uid)\n";
			std::cerr << "  -m,  --umask=UMASK              use this umask for daemon subprocess (default is 022)\n";
			std::cerr << "  -d,  --directory=DIRECTORY      directory to chdir to when daemonized\n";
			std::cerr << "  -l,  --logfile=FILENAME         use FILENAME as logfile path\n";
			std::cerr << "  -y,  --logfile_maxbytes=BYTES   use BYTES to limit the max size of logfile\n";
			std::cerr << "  -z,  --logfile_backups=NUM      number of backups to keep when max bytes reached\n";
			std::cerr << "  -e,  --loglevel=LEVEL           use LEVEL as log level (debug,info,warn,error,critical)\n";
			std::cerr << "  -j,  --pidfile=FILENAME         write a pid file for the daemon process to FILENAME\n";
			std::cerr << "  -i,  --identifier=STR           identifier used for this instance of supervisord\n";
			std::cerr << "  -q,  --childlogdir=DIRECTORY    the log directory for child process logs\n";
			std::cerr << "  -k,  --nocleanup                prevent the process from performing cleanup (removal of old automatic\n";
			std::cerr << "                                  child log files) at startup\n";
			std::cerr << "  -a,  --minfds=NUM               the minimum number of file descriptors for start success\n";
			std::cerr << "  -t,  --strip_ansi               strip ansi escape codes from process output\n";
			std::cerr << "  -p,  --minprocs=NUM             the minimum number of processes available for start success\n";
			std::cerr << "  -o,  --profile_options=OPTIONS  run supervisord under profiler and output results based on OPTIONS,\n";
			std::cerr << "                                  which  is a comma-sep'd list of 'cumulative', 'calls',\n";
			std::cerr << "                                  and/or 'callers', e.g. 'cumulative,callers')\n";
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

		int Options::log_level(const std::string &level) {
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

#pragma endregion

#pragma region "Parse"

	int Options::parse(int argc, char **argv) {
		_fullName = argv[0];

		struct option long_options[] = {
			{"configuration",		required_argument,	0, 'c'},	// [-c, --configuration=FILENAME]				- Ruta por defecto: /etc/supervisord.conf
			{"nodaemon",			no_argument,		0, 'n'},	// [-n, --nodaemon]								- Muestra log y sale con sañales
			{"silent",				no_argument,		0, 's'},	// [-s, --silent]								- 
			{"user",				required_argument,	0, 'u'},	// [-u, --user=USER]
			{"umask",				required_argument,	0, 'm'},	// [-m, --umask=UMASK]
			{"directory",			required_argument,	0, 'd'},	// [-d, --directory=DIRECTORY]
			{"logfile",				required_argument,	0, 'l'},	// [-l, --logfile=FILENAME]
			{"logfile_maxbytes",	required_argument,	0, 'y'},	// [-y, --logfile_maxbytes=BYTES]
			{"logfile_backups",		required_argument,	0, 'z'},	// [-z, --log-rotate-max=NUM]
			{"loglevel",			required_argument,	0, 'e'},	// [-e, --loglevel=LEVEL]
			{"pidfile",				required_argument,	0, 'j'},	// [-j, --pidfile=FILENAME]
			{"identifier",			required_argument,	0, 'i'},	// [-i, --identifier=STR]
			{"childlogdir",			required_argument,	0, 'q'},	// [-q, --childlogdir=DIRECTORY]
			{"nocleanup",			no_argument,		0, 'k'},	// [-k, --nocleanup]
			{"minfds",				required_argument,	0, 'a'},	// [-a, --minfds=NUM]
			{"strip_ansi",			no_argument,		0, 't'},	// [-t, --strip_ansi]
			{"minprocs",			required_argument,	0, 'p'},	// [-p, --minprocs=NUM]
			{"profile_options",		required_argument,	0, 'o'},	// [-o, --profile_options=OPTIONS]
			{"help",				no_argument,		0, 'h'},	// [-h, --help]
			{"version",				no_argument,		0, 'V'},	// [-V, --version]
			{0, 0, 0, 0}
		};

		int opt;
		while ((opt = getopt_long(argc, argv, "c:nsu:m:d:l:y:z:e:j:i:q:ka:tp:o:hv", long_options, NULL)) != -1) {
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
