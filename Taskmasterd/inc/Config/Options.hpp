/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Options.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/12 12:15:15 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/04 15:53:04 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Include"

	#include <string>															// std::string

#pragma endregion

#pragma region "ConfigOptions"

	class ConfigOptions {

		private:

			// Methods
			int	help();
			int	version();
			int	invalid();

		public:

			// Constructor
			ConfigOptions() = default;
			ConfigOptions(const ConfigOptions& src) = default;
			ConfigOptions(ConfigOptions&&) = default;
			~ConfigOptions() = default;

			// Overloads
			ConfigOptions& operator=(const ConfigOptions& rhs) = default;
			ConfigOptions& operator=(ConfigOptions&&) = default;

			// Variables
			std::string	configuration;											// Default path: /etc/taskmasterd.conf
			std::string	nodaemon;												// Shows log and exits on signals
			std::string	silent;													// Hides logs in stdout from debug to warning (only works with -n)
			std::string	user;													// Switch to this user after startup (privilege de-escalation) (requires root)
			std::string	umask;													// Set the file creation permission mask												(default: 022)
			std::string	directory;												// Set the initial working directory													(default: )
			std::string	logfile;												// File where the daemon writes its logs												(default: )
			std::string	logfile_maxbytes;										// Maximum log file size before rotation												(default: 10MB)
			std::string	logfile_backups;										// Number of backup files to keep during rotation										(default: 5)
			std::string	loglevel;												// Logging level: debug, info, warning, error, critical									(default: info)
			std::string	pidfile;												// File where the taskmaster process PID is written
			std::string	identifier;												// Unique identifier for this taskmaster instance (used in logs and communication)
			std::string	childlogdir;											// Directory where child processes write their logs by default
			std::string	strip_ansi;												// Remove ANSI escape sequences from child process logs
			std::string	nocleanup;												// Do not clean temporary files on exit
			std::string	minfds;													// Minimum number of file descriptors required											(default: 1024)
			std::string	minprocs;												// Minimum number of processes available in the system									(default: 200)

			std::string	fullName;												// Name and path used to execute the program (same as argv[0])
			std::string	options;												// 

			// Methods
			int	parse(int argc, char **argv);									// Parse options passed as arguments to the program
	};

#pragma endregion
