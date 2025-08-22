/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Options.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/12 12:15:15 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/22 17:40:00 by vzurera-         ###   ########.fr       */
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

			static std::string	configuration;									// Default path: /etc/taskmasterd.conf
			static bool			nodaemon;										// Shows log and exits on signals
			static bool			silent;											// Hides logs in stdout from debug to warning (only works with -n)
			static std::string	user;											// Switch to this user after startup (privilege de-escalation) (requires root)
			static uint8_t		umask;											// Set the file creation permission mask												(default: 022)
			static std::string	directory;										// Set the initial working directory													(default: )
			static std::string	logfile;										// File where the daemon writes its logs												(default: )
			static size_t		logfile_maxbytes;								// Maximum log file size before rotation												(default: 10MB)
			static uint8_t		logfile_backups;								// Number of backup files to keep during rotation										(default: 5)
			static uint8_t		loglevel;										// Logging level: debug, info, warning, error, critical									(default: info)
			static std::string	pidfile;										// File where the taskmaster process PID is written
			static std::string	identifier;										// Unique identifier for this taskmaster instance (used in logs and communication)
			static std::string	childlogdir;									// Directory where child processes write their logs by default
			static bool			nocleanup;										// Do not clean temporary files on exit
			static uint16_t		minfds;											// Minimum number of file descriptors required											(default: 1024)
			static bool			strip_ansi;										// Minimum number of processes available in the system									(default: 200)
			static uint16_t		minprocs;										// Remove ANSI escape sequences from child process logs

			static bool			is_root;										// 

			static int	parse(int argc, char **argv);							// Parse options passed as arguments to the program

		private:

			enum e_level { DEBUG, INFO, LOG, WARNING, ERROR, CRITICAL };

			static std::string	_fullName;										// Name and path used to execute the program (same as argv[0])

			Options() {}														// Default constructor (no instantiable)
			~Options() {}														// Destructor (no instantiable)
		
			template<typename T>
			static int			ft_strtoul(char **argv, const char *optarg, T *value, unsigned long max_value, bool allow_zero);
			static std::string	getTempPath();
			static int			check_fd_limit(uint16_t minfds);
			static int			check_process_limit(uint16_t minprocs);
			static int			validate_loglevel(const std::string &level);
			static int			validate_path(const std::string& level);
			static int			help();
			static int			version();
			static int			invalid();
	};

#pragma endregion
