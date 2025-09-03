/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Process.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/02 17:24:36 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/03 19:38:31 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include <cstdint>															// uint16_t, uint32_t
	#include <string>															// std::string
	#include <map>																// std::map
	#include <vector>															// std::vector

#pragma endregion

#pragma region "Process"

	class Program;
	class Process {

		public:

			// Enumerators
			enum { STOPPED, STARTING, RUNNING, BACKOFF, STOPPING, EXITED, FATAL, UNKNOWN };

			// Variables
			std::string							name;
			std::string							command;
			std::string							directory;
			std::string							user;
			uint16_t 							umask;
			uint16_t							priority;
			bool								autostart;
			uint8_t								autorestart;
			uint16_t							startsecs;
			uint8_t								startretries;
			std::vector<uint8_t>				exitcodes;
			uint8_t								stopsignal;
			uint16_t							stopwaitsecs;
			bool								stopasgroup;
			bool								killasgroup;
			bool								tty_mode;
			bool								redirect_stderr;
			std::string							stdout_logfile;
			uint32_t							stdout_logfile_maxbytes;
			uint16_t							stdout_logfile_backups;
			bool								stdout_logfile_syslog;
			std::string							stderr_logfile;
			uint32_t							stderr_logfile_maxbytes;
			uint16_t							stderr_logfile_backups;
			bool								stderr_logfile_syslog;
			std::map<std::string, std::string>	environment;

			pid_t								pid;
			uint16_t							process_num;
			uint8_t								status;
			time_t								start_time;
			time_t								stop_time;
			time_t								change_time;
			time_t								uptime;
			uint32_t							restart_count;
			uint16_t							killwait_secs;
			int									exit_code;
			std::string							exit_reason;
			std::string							spawn_error;
			std::string							program_name;
			Program								*program;

			// Constructors
			Process();
			Process(const Process&) = default;
			~Process() = default;

			// Overloads
			Process& operator=(const Process&) = default;

		private:

	};

#pragma endregion
