/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Options.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/12 12:15:15 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/21 16:12:00 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Include"

	#include <string>															// std::string
	#include <cstdint>															// uint8_t, uint16_t

#pragma endregion

#pragma region "Defines"

	#define NAME	"taskmasterd"												// Name of the program

#pragma endregion

#pragma region "Options"

	class Options {

		public:

			static bool			disabledEncryption;								// Disable encrypted communication 
			static bool			disabledShell;									// Disable remote shell
			static uint16_t		maxClients;										// Maximum number of clients connected simultaneously
			static uint16_t		portNumber;										// Port to listen for incoming connections
			static uint16_t		timeout;										// Timeout in seconds for inactive connections
			static std::string	logFile;										// Path for the log file
			static uint8_t		logLevel;										// Logging level
			static bool			logNew;											// Create a new log file on start
			static uint8_t		logMax;											// Maximum number of log files to keep when rotating
			static size_t		logSize;										// Minimum log size before rotation
			static std::string	shellPath;										// Path of the shell to execute

			static int			signum;											// Signal value when a signal is intercepted
			static int			lockfd;											// FD for daemon lock
			static int			sockfd;											// FD for the socket

			static int	parse(int argc, char **argv);							// Parse options passed as arguments to the program

		private:

			enum e_level { DEBUG, INFO, LOG, WARNING, ERROR, CRITICAL };

			static std::string	_fullName;										// Name and path used to execute the program (same as argv[0])

			Options() {}														// Default constructor (no instantiable)
			~Options() {}														// Destructor (no instantiable)
		
			template<typename T>
			static int	ft_strtoul(char **argv, const char *optarg, T *value, unsigned long max_value, bool allow_zero);
			static int	validate_loglevel(const std::string &level);
			static int	validate_path(const std::string& level);
			static int	help();
			static int	version();
			static int	invalid();
	};

#pragma endregion
