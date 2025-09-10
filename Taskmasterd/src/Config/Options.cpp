/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Options.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/12 12:15:32 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/10 18:35:56 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Options.hpp"
	#include "Config/Config.hpp"

	#include <cstring>															// strerror()
	#include <iostream>															// std::cerr()
	#include <getopt.h>															// getopt_long()

#pragma endregion

#pragma region "Validate"

	int ConfigOptions::validate_options() const {
		static std::string	dir;
		std::string			errors;

		if (options.find_first_of('d') != std::string::npos) {
			if (!Utils::valid_path(directory, dir, true))
				errors += "directory:\t\tpath is invalid - " + std::string(strerror(errno)) + "\n";
			dir = Utils::expand_path(directory, "", true, false);
		}

		if (options.find_first_of('c') != std::string::npos) {
			if (!Utils::valid_path(configuration, dir) || Utils::expand_path(configuration, dir, true, false).empty())
				errors += "configuration:\t\tpath is invalid - " + std::string(strerror(errno)) + "\n";
		}

		if (options.find_first_of('u') != std::string::npos) {
			if (!Utils::valid_user(user))
				errors += "user:\t\t\tinvalid user\n";
		}

		if (options.find_first_of('m') != std::string::npos) {
			if (!Utils::valid_umask(umask))
				errors += "umask:\t\t\tmust be in octal format\n";
		}

		if (options.find_first_of('l') != std::string::npos) {
			if (!Utils::valid_path(logfile, dir, false, false, true) || Utils::expand_path(logfile, dir).empty())
				errors += "logfile:\t\tpath is invalid - " + std::string(strerror(errno)) + "\n";
		}

		if (options.find_first_of('y') != std::string::npos) {
			long bytes = Utils::parse_size(logfile_maxbytes);
			if (bytes == -1 || !Utils::valid_number(std::to_string(bytes), 0, 1024 * 1024 * 1024))
				errors += "logfile_maxbytes:\tmust be a value between 0 bytes and 1024 MB\n";
		}

		if (options.find_first_of('z') != std::string::npos) {
			if (!Utils::valid_number(logfile_backups, 0, 1000))
				errors += "logfile_backups:\tmust be a value between 0 and 1000\n";
		}

		if (options.find_first_of('e') != std::string::npos) {
			if (!Utils::valid_loglevel(loglevel))
				errors += "loglevel:\t\tmust be one of: DEBUG, INFO, WARNING, ERROR, CRITICAL\n";
		}

		if (options.find_first_of('j') != std::string::npos) {
			if (!Utils::valid_path(pidfile, dir, false, false, true) || Utils::expand_path(pidfile, dir).empty())
				errors += "pidfile:\t\tpath is invalid - " + std::string(strerror(errno)) + "\n";
		}

		if (options.find_first_of('q') != std::string::npos) {
			if (!Utils::valid_path(childlogdir, dir, true))
				errors += "childlogdir:\t\tpath is invalid - " + std::string(strerror(errno)) + "\n";
		}

		if (options.find_first_of('a') != std::string::npos) {
			if (!Utils::valid_number(minfds, 1, 65535))
				errors += "minfds:\t\t\tmust be a value between 1 and 65535\n";
		}

		if (options.find_first_of('p') != std::string::npos) {
			if (!Utils::valid_number(minprocs, 1, 65535))
				errors += "minprocs:\t\tmust be a value between 1 and 10000\n";
		}

		if (!errors.empty()) { std::cerr << fullName << ": invalid options: \n\n" <<  errors; return (2); }

		return (0);
	}

#pragma endregion
	
#pragma region "Messages"

	#pragma region "Help"

		int ConfigOptions::help() const {
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
			std::cerr << "  -e,  --loglevel=LEVEL           Use LEVEL as log level (DEBUG, INFO, WARNING, ERROR, CRITICAL)\n";
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
			std::cerr << "Report bugs to <kobayashi82@outlook.com>" << std::endl;

			return (1);
		}

	#pragma endregion

	#pragma region "Version"

		int ConfigOptions::version() const {
			std::cerr << "taskmasterd: 1.0 part of Taskmaster\n";
			std::cerr << "Copyright (C) 2025 Kobayashi Corp ⓒ.\n";
			std::cerr << "\n";
			std::cerr << "License WTFPL: DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE.\n";
			std::cerr << "This is free software: you are free to change and redistribute it.\n";
			std::cerr << "There is NO WARRANTY, to the extent permitted by law.\n";
			std::cerr << "\n";
			std::cerr << "Written by Kobayashi82 (vzurera-)." << std::endl;

			return (1);
		}

	#pragma endregion

	#pragma region "Invalid"

		int ConfigOptions::invalid() const {
			std::cerr << "Try '" << fullName << " --help' for more information." << std::endl;
			return (2);
		}

	#pragma endregion

#pragma endregion

#pragma region "Parse"

	int ConfigOptions::parse(int argc, char **argv) {
		fullName = argv[0];

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
		while ((opt = getopt_long(argc, argv, "c:nsu:m:d:l:y:z:e:j:i:q:tka:p:h?v", long_options, NULL)) != -1) {
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
			std::cerr << "taskmasterd: invalid argument: " << argv[optind] << std::endl;
			invalid(); return (2);
		}

		if (validate_options()) { std::cerr << "\n"; invalid(); return (2); }

		return (0);
	}

#pragma endregion
