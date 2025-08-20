/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Options.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/12 12:15:32 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/18 22:20:09 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Taskmaster/Config/Options.hpp"

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
			std::cerr << "  -k,  --disable-encryption   Disable encryption communication for Ben_AFK clients\n";
			std::cerr << "  -s,  --disable-shell        Disable remote shell access\n";
			std::cerr << "  -c,  --max-clients=NUM      Maximum number of clients                           (default: 3, unlimited = 0)\n";
			std::cerr << "  -p,  --port=PORT            Port number to listen on                            (default: 4242)\n";
			std::cerr << "  -t,  --timeout=SECOND       Timeout in seconds for inactive connections         (default: 600)\n";
			std::cerr << "  -f,  --log-file=PATH        Path to the log file                                (default: /var/log/matt_daemon/matt_daemon.log)\n";
			std::cerr << "  -l,  --log-level=LEVEL      Logging verbosity level                             (default: INFO)\n";
			std::cerr << "  -n,  --log-new              Create a new log file on start\n";
			std::cerr << "  -m,  --log-rotate-max=NUM   Maximum number of log files to keep when rotating   (default: 5)\n";
			std::cerr << "  -r,  --log-rotate-size=BYTE Minimum log size before rotation                    (default: 10M\n";
			std::cerr << "  -x,  --shell-path=PATH      Path of the shell to execute\n";
			std::cerr << "\n";
			std::cerr << "  -h?, --help                 Display this help message\n";
      		std::cerr << "  -u,  --usage                Display short usage message\n";
  			std::cerr << "  -V,  --version              Show program version\n";
			std::cerr << "\n";
			std::cerr << "Report bugs to <kobayashi82@outlook.com>\n";

			return (1);
		}

	#pragma endregion

	#pragma region "Usage"

		int Options::usage() {
			std::cerr << "Usage: " << NAME << " [-k, --disable-encryption] [-s, --disable-shell] [-c NUM, --max-clients=NUM] [-p PORT, --port=PORT]\n";
			std::cerr << "                  [-t SECOND, --timeout=SECOND] [-f PATH, --log-file=PATH] [-l LEVEL, --log-level=LEVEL] [-n, --log-new]\n";
			std::cerr << "                  [-m NUM, --log-rotate-max=NUM] [-r BYTE, --log-rotate-size=BYTE] [-x, --shell-path=PATH]\n";
			std::cerr << "                  [-h? --help] [-u --usage] [-V --version]\n";

			return (1);
		}

	#pragma endregion

	#pragma region "Version"

		int Options::version() {
			std::cerr << NAME << " 1.0\n";
			std::cerr << "Copyright (C) 2025 Kobayashi Corp ⓒ.\n";
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
			std::cerr << "Try '" << _fullName << " --help' or '" << _fullName << " --usage' for more information.\n";
			return (2);
		}

	#pragma endregion

	#pragma region "Log Level"

		int Options::log_level(const std::string &level) {
			std::string l = level;
			std::transform(l.begin(), l.end(), l.begin(), ::tolower);

			if		(l == "debug")		logLevel = DEBUG;
			else if	(l == "info")		logLevel = INFO;
			else if	(l == "log")		logLevel = LOG;
			else if	(l == "warning")	logLevel = WARNING;
			else if	(l == "error")		logLevel = ERROR;
			else if	(l == "critical")	logLevel = CRITICAL;
			else {
				std::cerr << "Invalid log level. Valid values are: DEBUG, INFO, LOG, WARNING, CRITICAL\n";
				return (1);
			}

			return (0);
		}

	#pragma endregion

#pragma endregion

#pragma region "Parse"

	int Options::parse(int argc, char **argv) {
		_fullName = argv[0];

		struct option long_options[] = {
			{"disable-encryption",	no_argument,		0, 'k'},	// [-k, --disable-encryption]
			{"disable-shell",		no_argument,		0, 's'},	// [-s, --disable-shell]
			{"max-clients",			required_argument,	0, 'c'},	// [-c, --max-clients=NUM]
			{"port",				required_argument,	0, 'p'},	// [-p, --port=NUM]
			{"timeout",				required_argument,	0, 't'},	// [-t, --timeout=SEC]
			{"log-file",			required_argument,	0, 'f'},	// [-c, --log-file=PATH]
			{"log-level",			required_argument,	0, 'l'},	// [-c, --log-level=LEVEL]
			{"log-new",				no_argument,		0, 'n'},	// [-n, --log-new]
			{"log-rotate-max",		required_argument,	0, 'm'},	// [-m, --log-rotate-max=NUM]
			{"log-rotate-size",		required_argument,	0, 'r'},	// [-r, --log-rotate-size=NUM]
			{"shell-path",			required_argument,	0, 'x'},	// [-x, --shell-path=PATH]

			{"help",				no_argument,		0, 'h'},	// [-h?, --help]
			{"usage",				no_argument,		0, 'u'},	// [	--usage]
			{"version",				no_argument,		0, 'V'},	// [-V, --version]
			{0, 0, 0, 0}
		};

		int opt;
		while ((opt = getopt_long(argc, argv, "ksc:p:t:f:l:nm:r:x:h?uV", long_options, NULL)) != -1) {
			switch (opt) {
				case 'k':	disabledEncryption = true;																	break;
				case 's':	disabledShell = true;																		break;
				case 'c':	if (ft_strtoul(argv, optarg, &maxClients, 1024 , true))				return (2);				break;
				case 'p':	if (ft_strtoul(argv, optarg, &portNumber, 65535, false))			return (2);				break;
				case 't':	if (ft_strtoul(argv, optarg, &timeout, 65535, true))				return (2);				break;
				case 'f':	logFile = std::string(optarg);																break;
				case 'l':	if (log_level(std::string(optarg)))									return (2);				break;
				case 'n':	logNew = true;																				break;
				case 'm':	if (ft_strtoul(argv, optarg, &logMax, 256, true))					return (2);				break;
				case 'r':
				{
					std::string value = std::string(optarg);
					int multiplier = 1;
					if (!value.empty() && (value.back() == 'k' || value.back() == 'K')) { multiplier = 1024;		value.pop_back(); }
					if (!value.empty() && (value.back() == 'm' || value.back() == 'M')) { multiplier = 1024 * 1024;	value.pop_back(); }
					if (ft_strtoul(argv, value.c_str(), &logSize, 1024 * 1024 * 1024, true))	return (2);
					logSize *= multiplier;																				break;
				}
				case 'x':	shellPath = std::string(optarg);															break;

				case '?':	if (std::string(argv[optind - 1]) == "-?")							return (help());		return (invalid());
				case 'h':																		return (help());
				case 'u':																		return (usage());
				case 'V':																		return (version());
			}
		}

		if (optind < argc) {
			std::cerr << NAME << ": invalid argument: " << argv[optind] << "\n";
			invalid(); return (2);
		}

		return (0);
	}

#pragma endregion
