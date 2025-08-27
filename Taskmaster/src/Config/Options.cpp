/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Options.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/12 12:15:32 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/27 12:13:42 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Options.hpp"
	#include "Config/Parser.hpp"

	#include <unistd.h>															// getuid()
	#include <iostream>															// std::cerr()
	#include <getopt.h>															// getopt_long()

#pragma endregion

#pragma region "Constructors"

	#pragma region "Constructor"

		ConfigOptions::ConfigOptions() :
			_fullName			("taskmasterd"),								// Name and path used to execute the program (same as argv[0])
			configuration		(Parser.config_path()),							// Default path: /etc/taskmasterd.conf
			nodaemon			("false"),										// Shows log and exits on signals
			silent				("false"),										// Hides logs in stdout from debug to warning (only works with -n)
			user				("do not switch"),								// Switch to this user after startup (privilege de-escalation) (requires root)
			umask				("022"),										// Set the file creation permission mask												(default: 022)
			directory			("do not change"),								// Set the initial working directory													(default: )
			logfile				(Parser.expand_path("supervisord.log")),		// File where the daemon writes its logs												(default: )
			logfile_maxbytes	("50 MB"),										// Maximum log file size before rotation												(default: 10MB)
			logfile_backups		("10"),											// Number of backup files to keep during rotation										(default: 5)
			loglevel			("info"),										// Logging level: debug, info, warning, error, critical									(default: info)
			pidfile				(Parser.expand_path("supervisord.pid")),		// File where the taskmaster process PID is written
			identifier			("taskmaster"),									// Unique identifier for this taskmaster instance (used in logs and communication)
			childlogdir			(Parser.temp_path()),							// Directory where child processes write their logs by default
			strip_ansi			("false"),										// Remove ANSI escape sequences from child process logs
			nocleanup			("false"),										// Do not clean temporary files on exit
			minfds				("1024"),										// Minimum number of file descriptors required											(default: 1024)
			minprocs			("200"),										// Minimum number of processes available in the system									(default: 200)
			options				(""),											// 
			is_root				(getuid() == 0)									// 
		{}

	#pragma endregion

	#pragma region "Constructor (copy)"

		ConfigOptions::ConfigOptions(const ConfigOptions& src) :
			_fullName			(src._fullName),								// Name and path used to execute the program (same as argv[0])
			configuration		(src.configuration),							// Default path: /etc/taskmasterd.conf
			nodaemon			(src.nodaemon),									// Shows log and exits on signals
			silent				(src.silent),									// Hides logs in stdout from debug to warning (only works with -n)
			user				(src.user),										// Switch to this user after startup (privilege de-escalation) (requires root)
			umask				(src.umask),									// Set the file creation permission mask												(default: 022)
			directory			(src.directory),								// Set the initial working directory													(default: )
			logfile				(src.logfile),									// File where the daemon writes its logs												(default: )
			logfile_maxbytes	(src.logfile_maxbytes),							// Maximum log file size before rotation												(default: 10MB)
			logfile_backups		(src.logfile_backups),							// Number of backup files to keep during rotation										(default: 5)
			loglevel			(src.loglevel),									// Logging level: debug, info, warning, error, critical									(default: info)
			pidfile				(src.pidfile),									// File where the taskmaster process PID is written
			identifier			(src.identifier),								// Unique identifier for this taskmaster instance (used in logs and communication)
			childlogdir			(src.childlogdir),								// Directory where child processes write their logs by default
			strip_ansi			(src.strip_ansi),								// Remove ANSI escape sequences from child process logs
			nocleanup			(src.nocleanup),								// Do not clean temporary files on exit
			minfds				(src.minfds),									// Minimum number of file descriptors required											(default: 1024)
			minprocs			(src.minprocs),									// Minimum number of processes available in the system									(default: 200)
			options				(src.options),									// 
			is_root				(src.is_root)									// 
		{}

	#pragma endregion

#pragma endregion

#pragma region "Overloads"

	#pragma region "Assing"

	ConfigOptions& ConfigOptions::operator=(const ConfigOptions& rhs) {
		if (this == &rhs) return (*this);

		_fullName			= rhs._fullName;									// Name and path used to execute the program (same as argv[0])
		configuration		= rhs.configuration;								// Default path: /etc/taskmasterd.conf
		nodaemon			= rhs.nodaemon;										// Shows log and exits on signals
		silent				= rhs.silent;										// Hides logs in stdout from debug to warning (only works with -n)
		user				= rhs.user;											// Switch to this user after startup (privilege de-escalation) (requires root)
		umask				= rhs.umask;										// Set the file creation permission mask												(default: 022)
		directory			= rhs.directory;									// Set the initial working directory													(default: )
		logfile				= rhs.logfile;										// File where the daemon writes its logs												(default: )
		logfile_maxbytes	= rhs.logfile_maxbytes;								// Maximum log file size before rotation												(default: 10MB)
		logfile_backups		= rhs.logfile_backups;								// Number of backup files to keep during rotation										(default: 5)
		loglevel			= rhs.loglevel;										// Logging level: debug, info, warning, error, critical									(default: info)
		pidfile				= rhs.pidfile;										// File where the taskmaster process PID is written
		identifier			= rhs.identifier;									// Unique identifier for this taskmaster instance (used in logs and communication)
		childlogdir			= rhs.childlogdir;									// Directory where child processes write their logs by default
		strip_ansi			= rhs.strip_ansi;									// Remove ANSI escape sequences from child process logs
		nocleanup			= rhs.nocleanup;									// Do not clean temporary files on exit
		minfds				= rhs.minfds;										// Minimum number of file descriptors required											(default: 1024)
		minprocs			= rhs.minprocs;										// Minimum number of processes available in the system									(default: 200)
		options				= rhs.options;										// 
		is_root				= rhs.is_root;										// 

		return (*this);
	}

#pragma endregion

#pragma endregion

#pragma region "Utils"

	#pragma region "Strtoul"

		// template<typename T>
		// int ConfigOptions::ft_strtoul(char **argv, const char *optarg, T *value, unsigned long max_value, bool allow_zero) {
		// 	try {
		// 		size_t			idx;
		// 		unsigned long	tmp = std::stoul(optarg, &idx, 0);

		// 		if (idx != std::strlen(optarg)) {
		// 			std::cerr << argv[0] << ": invalid value (`" << optarg << "' near `" << (optarg + idx) << "')\n";
		// 			return (1);
		// 		}

		// 		if (!tmp && !allow_zero) {
		// 			std::cerr << argv[0] << ": option value too small: " << optarg << "\n";
		// 			return (1);
		// 		}

		// 		if (max_value && tmp > max_value) {
		// 			std::cerr << argv[0] << ": option value too big: " << optarg << "\n";
		// 			return (1);
		// 		}

		// 		*value = static_cast<T>(tmp);
		// 		return (0);
		// 	} catch (const std::exception &) {
		// 		std::cerr << argv[0] << ": invalid number: " << optarg << "\n";
		// 		return (1);
		// 	}
		// }

	#pragma endregion

#pragma endregion

#pragma region "Messages"

	#pragma region "Help"

		int ConfigOptions::help() {
			std::cerr << "Usage: " << "taskmasterd: [ OPTION... ] \n";
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

		int ConfigOptions::version() {
			std::cerr << "taskmasterd: 1.0 part of Taskmaster\n";
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

		int ConfigOptions::invalid() {
			std::cerr << "Try '" << _fullName << " --help' for more information.\n";
			return (2);
		}

	#pragma endregion

	#pragma region "Log Level"

		// int ConfigOptions::validate_loglevel(const std::string& level) {
		// 	std::string l = level;
		// 	std::transform(l.begin(), l.end(), l.begin(), ::tolower);

		// 	if		(l == "debug")		loglevel = Config::DEBUG;
		// 	else if	(l == "info")		loglevel = Config::INFO;
		// 	else if	(l == "warning")	loglevel = Config::WARNING;
		// 	else if	(l == "warn")		loglevel = Config::WARNING;
		// 	else if	(l == "error")		loglevel = Config::ERROR;
		// 	else if	(l == "critical")	loglevel = Config::CRITICAL;
		// 	else { std::cerr << "Invalid log level. Valid values are: DEBUG, INFO, WARNING, CRITICAL\n"; return (1); }

		// 	return (0);
		// }

	#pragma endregion

	// int ConfigOptions::validate_path(const std::string& path) {
	// 	(void) path;
	// 	// Convertir ~
	// 	return (0);
	// }

#pragma endregion

#pragma region "Parse"

	int ConfigOptions::parse(int argc, char **argv) {
		_fullName = argv[0];

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
			{"strip_ansi",			no_argument,		0, 't'},	// [-t, --strip_ansi]							- Minimum number of processes available in the system								(default: 200)
			{"nocleanup",			no_argument,		0, 'k'},	// [-k, --nocleanup]							- Do not clean temporary files on exit
			{"minfds",				required_argument,	0, 'a'},	// [-a, --minfds=NUM]							- Minimum number of file descriptors required										(default: 1024)
			{"minprocs",			required_argument,	0, 'p'},	// [-p, --minprocs=NUM]							- Remove ANSI escape sequences from child process logs
			{"help",				no_argument,		0, 'h'},	// [-h, --help]
			{"version",				no_argument,		0, 'V'},	// [-V, --version]
			{0, 0, 0, 0}
		};

		int opt;
		while ((opt = getopt_long(argc, argv, "c:nsu:m:d:l:y:z:e:j:i:q:tka:p:hv", long_options, NULL)) != -1) {
			switch (opt) {
				case 'c':	{ options += opt; configuration		= std::string(optarg);		break; }
				case 'n':	{ options += opt; nodaemon			= "true";					break; }
				case 's':	{ options += opt; silent			= "true";					break; }
				case 'u':	{ options += opt; user				= std::string(optarg);		break; }
				case 'm':	{ options += opt; umask				= std::string(optarg);		break; }
				case 'd':	{ options += opt; directory			= std::string(optarg);		break; }
				case 'l':	{ options += opt; logfile			= std::string(optarg);		break; }
				case 'y':	{ options += opt; logfile_maxbytes	= std::string(optarg);		break; }
				case 'z':	{ options += opt; logfile_backups	= std::string(optarg);		break; }
				case 'e':	{ options += opt; loglevel			= std::string(optarg);		break; }
				case 'j':	{ options += opt; pidfile			= std::string(optarg);		break; }
				case 'i':	{ options += opt; identifier		= std::string(optarg);		break; }
				case 'q':	{ options += opt; childlogdir		= std::string(optarg);		break; }
				case 't':	{ options += opt; strip_ansi		= "true";					break; }
				case 'k':	{ options += opt; nocleanup			= "true";					break; }
				case 'a':	{ options += opt; minfds			= std::string(optarg);		break; }
				case 'p':	{ options += opt; minprocs			= std::string(optarg);		break; }

				case '?':	if (std::string(argv[optind - 1]) != "-?") return (invalid());	return (help());
				case 'h':																	return (help());
				case 'v':																	return (version());
			}
		}

		if (optind < argc) {
			std::cerr << "taskmasterd: invalid argument: " << argv[optind] << "\n";
			invalid(); return (2);
		}

		return (0);
	}

#pragma endregion
