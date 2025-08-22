/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Options.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/12 12:15:32 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/22 17:39:52 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Options.hpp"

	#include <cstring>															// std::strlen()
	#include <iostream>															// std::cerr()
	#include <algorithm>														// std::transform()
	#include <filesystem>														// std::filesystem::temp_directory_path()

	#include <unistd.h>															// getuid()
	#include <getopt.h>															// getopt_long()
	#include <sys/resource.h>													// getrlimit(), setrlimit()

#pragma endregion

#pragma region "Variables"

	std::string	Options::configuration		= "";								// Default path: /etc/taskmasterd.conf
	bool		Options::nodaemon			= false;							// Shows log and exits on signals
	bool		Options::silent				= false;							// Hides logs in stdout from debug to warning (only works with -n)
	std::string	Options::user				= "";								// Switch to this user after startup (privilege de-escalation) (requires root)
	uint8_t		Options::umask				= 022;								// Set the file creation permission mask												(default: 022)
	std::string	Options::directory			= "";								// Set the initial working directory													(default: )
	std::string	Options::logfile			= "supervisord.log";				// File where the daemon writes its logs												(default: )
	size_t		Options::logfile_maxbytes	= 50 * 1024 * 1024;					// Maximum log file size before rotation												(default: 10MB)
	uint8_t		Options::logfile_backups	= 10;								// Number of backup files to keep during rotation										(default: 5)
	uint8_t		Options::loglevel			= INFO;								// Logging level: debug, info, warning, error, critical									(default: info)
	std::string	Options::pidfile			= "supervisord.pid";				// File where the taskmaster process PID is written
	std::string	Options::identifier			= "taskmaster";						// Unique identifier for this taskmaster instance (used in logs and communication)
	std::string	Options::childlogdir		= "";								// Directory where child processes write their logs by default
	bool		Options::nocleanup			= false;							// Do not clean temporary files on exit
	uint16_t	Options::minfds				= 1024;								// Minimum number of file descriptors required											(default: 1024)
	bool		Options::strip_ansi			= true;								// Minimum number of processes available in the system									(default: 200)
	uint16_t	Options::minprocs			= 200;								// Remove ANSI escape sequences from child process logs

	bool		Options::is_root			= false;							// Minimum number of processes available in the system									(default: 200)

	std::string	Options::_fullName			= "taskmasterd";					// Name and path used to execute the program (same as argv[0])

#pragma endregion

#pragma region "Utils"

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

	#pragma region "Get Temp Path"

		std::string Options::getTempPath() {
			try {
				return (std::filesystem::temp_directory_path().string());
			} catch (const std::filesystem::filesystem_error& e) {
				return ("");
			}
		}

	#pragma endregion

	#pragma region "Check FD Limit"

		int Options::check_fd_limit(uint16_t minfds) {
			struct rlimit rl;

			if (!getrlimit(RLIMIT_NOFILE, &rl)) {
				if (rl.rlim_cur > minfds) return (0);
				else if (is_root && rl.rlim_cur < rl.rlim_max && minfds <= rl.rlim_max) {
					rl.rlim_cur = minfds;
					return (setrlimit(RLIMIT_NOFILE, &rl) != 0);
				}
			}

			return (1);
		}

	#pragma endregion

	#pragma region "Check Process Limit"

		int Options::check_process_limit(uint16_t minprocs) {
			struct rlimit rl;

			if (getrlimit(RLIMIT_NPROC, &rl) == 0) {
				if (rl.rlim_cur > minprocs) return (0);
				else if (is_root && rl.rlim_cur < rl.rlim_max && minprocs <= rl.rlim_max) {
					rl.rlim_cur = minprocs;
					return (setrlimit(RLIMIT_NOFILE, &rl) != 0);
				}
			}

			return (1);
		}

	#pragma endregion

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

			if		(l == "debug")		loglevel = DEBUG;
			else if	(l == "info")		loglevel = INFO;
			else if	(l == "warning")	loglevel = WARNING;
			else if	(l == "warn")		loglevel = WARNING;
			else if	(l == "error")		loglevel = ERROR;
			else if	(l == "critical")	loglevel = CRITICAL;
			else { std::cerr << "Invalid log level. Valid values are: DEBUG, INFO, WARNING, CRITICAL\n"; return (1); }

			return (0);
		}

	#pragma endregion

	int Options::validate_path(const std::string& path) {
		(void) path;
		// Convertir ~
		return (0);
	}

#pragma endregion

#pragma region "Parse"

	int Options::parse(int argc, char **argv) {
		_fullName = argv[0];
		is_root = getuid() == 0;

		struct option long_options[] = {
			{"configuration",		required_argument,	0, 'c'},	// [-c, --configuration=FILENAME]				- Default path: /etc/taskmasterd.conf
			{"nodaemon",			no_argument,		0, 'n'},	// [-n, --nodaemon]								- Shows log and exits on signals
			{"silent",				no_argument,		0, 's'},	// [-s, --silent]								- Hides logs in stdout from debug to warning (only works with -n)
			{"user",				required_argument,	0, 'u'},	// [-u, --user=USER]							- Switch to this user after startup (privilege de-escalation) (requires root)
			{"umask",				required_argument,	0, 'm'},	// [-m, --umask=UMASK]							- Set the file creation permission mask												(default: 022)
			{"directory",			required_argument,	0, 'd'},	// [-d, --directory=DIRECTORY]					- Set the initial working directory													(default: )
			{"logfile",				required_argument,	0, 'l'},	// [-l, --logfile=FILENAME]						- File where the daemon writes its logs												(default: )
			{"logfile_maxbytes",	required_argument,	0, 'y'},	// [-y, --logfile_maxbytes=BYTES]				- Maximum log file size before rotation												(default: 10MB)
			{"logfile_backups",		required_argument,	0, 'z'},	// [-z, --logfile_backups=NUM]					- Number of backup files to keep during rotation									(default: 5)
			{"loglevel",			required_argument,	0, 'e'},	// [-e, --loglevel=LEVEL]						- Logging level: debug, info, warning, error, critical								(default: info)
			{"pidfile",				required_argument,	0, 'j'},	// [-j, --pidfile=FILENAME]						- File where the taskmaster process PID is written
			{"identifier",			required_argument,	0, 'i'},	// [-i, --identifier=STR]						- Unique identifier for this taskmaster instance (used in logs and communication)
			{"childlogdir",			required_argument,	0, 'q'},	// [-q, --childlogdir=DIRECTORY]				- Directory where child processes write their logs by default
			{"nocleanup",			no_argument,		0, 'k'},	// [-k, --nocleanup]							- Do not clean temporary files on exit
			{"minfds",				required_argument,	0, 'a'},	// [-a, --minfds=NUM]							- Minimum number of file descriptors required										(default: 1024)
			{"strip_ansi",			no_argument,		0, 't'},	// [-t, --strip_ansi]							- Minimum number of processes available in the system								(default: 200)
			{"minprocs",			required_argument,	0, 'p'},	// [-p, --minprocs=NUM]							- Remove ANSI escape sequences from child process logs
			{"help",				no_argument,		0, 'h'},	// [-h, --help]
			{"version",				no_argument,		0, 'V'},	// [-V, --version]
			{0, 0, 0, 0}
		};

		int opt;
		while ((opt = getopt_long(argc, argv, "c:nsu:m:d:l:y:z:e:j:i:q:ka:tp:hv", long_options, NULL)) != -1) {
			switch (opt) {
				case 'c':	configuration = std::string(optarg);														break;
				case 'n':	nodaemon = true;																			break;
				case 's':	silent = true;																				break;
				case 'u':	user = std::string(optarg);																	break;
				case 'm':	umask = 022;																				break;
				case 'd':	directory = std::string(optarg);															break;
				case 'l':	logfile = std::string(optarg);																break;
				case 'y':	if (ft_strtoul(argv, optarg, &logfile_maxbytes, 1024 , true))		return (2);				break;
				case 'z':	if (ft_strtoul(argv, optarg, &logfile_backups, 255 , true))			return (2);				break;
				case 'e':	if (validate_loglevel(optarg))										return (2);				break;
				case 'j':	pidfile = std::string(optarg);																break;
				case 'i':	identifier = std::string(optarg);															break;
				case 'q':	childlogdir = std::string(optarg);															break;
				case 'k':	nocleanup = true;																			break;
				case 'a':	if (ft_strtoul(argv, optarg, &minfds, 1024 , true))					return (2);				break;
				case 't':	strip_ansi = true;																			break;
				case 'p':	if (ft_strtoul(argv, optarg, &minprocs, 255 , true))				return (2);				break;

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
