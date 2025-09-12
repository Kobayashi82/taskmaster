/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Process.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/02 17:24:36 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/12 12:21:08 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include <string>															// std::string
	#include <cstdint>															// uint16_t, uint32_t
	#include <map>																// std::map
	#include <deque>															// std::deque
	#include <vector>															// std::vector
	#include <chrono>															// std::time_t

#pragma endregion

#pragma region "Enumerators"

	enum class ProcessState { STOPPED, STARTING, RUNNING, BACKOFF, STOPPING, EXITED, FATAL, UNKNOWN };

#pragma endregion

#pragma region "Status Event"

	struct StatusEvent {
		// Variables
		std::string						name;
		std::string						program_name;
		ProcessState					status;
		int								exit_code;
		std::time_t						event_time;

		// Constructors
		StatusEvent() = delete;
		StatusEvent(const std::string& _name, const std::string& _program_name, ProcessState _status, int _exit_code) : name(_name), program_name(_program_name), status(_status), exit_code(_exit_code), event_time(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())) {}
		StatusEvent(const StatusEvent&) = default;
		StatusEvent(StatusEvent&&) = default;
		~StatusEvent() = default;

		// Overloads
		StatusEvent& operator=(const StatusEvent&) = default;
		StatusEvent& operator=(StatusEvent&&) = default;

		// Methods
		std::string	to_string() const;
	};

#pragma endregion

#pragma region "Process"

	class Program;
	class Process {

		public:

			// Constructors
			Process();
			Process(const Process&) = default;
			Process(Process&&) = default;
			~Process() = default;

			// Overloads
			Process& operator=(const Process&) = default;
			Process& operator=(Process&&) = default;

			// Variables
			std::string							name;
			std::string							command;
			std::vector<std::string>			arguments;
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
			std::string							serverurl;
			std::map<std::string, std::string>	environment;

			pid_t								pid;
			uint16_t							process_num;
			ProcessState						status;
			std::time_t							start_time;
			std::time_t							stop_time;
			std::time_t							change_time;
			std::time_t							uptime;
			uint32_t							restart_count;
			uint16_t							killwait_secs;
			int									exit_code;
			std::string							exit_reason;
			std::string							spawn_error;
			std::string							program_name;
			bool								terminated;
			int									std_in;
			int									std_out;
			int									std_err;
			std::deque<StatusEvent>				history;

			void		history_add();
			std::string	history_get(uint16_t tail);
			void		history_clear();

		private:

			uint16_t MAX_HISTORY_SIZE = 500;
	};

#pragma endregion
