/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TaskmasterLog.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/11 22:28:53 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/04 11:53:37 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Utils/Utils.hpp"
	#include "Config/Config.hpp"
	#include "Logging/TaskmasterLog.hpp"

	#include <cstring>															// strerror()
	#include <iostream>															// std::cout
	#include <chrono>															// 

#pragma endregion

#pragma region "Variables"

	TaskmasterLog Log;

#pragma endregion

#pragma region "Constructors"

	TaskmasterLog::TaskmasterLog() :
		_logfile(""),
		_logfile_maxbytes(0),
		_logfile_backups(0),
		_logfile_level(DEBUG),
		_logfile_syslog(false),
		_logfile_stdout(false),
		_logfile_ready(false)
	{}

	TaskmasterLog::~TaskmasterLog() {
		
	}

#pragma endregion

#pragma region "Open"

	int TaskmasterLog::open() {
		if (_logfile.empty() || Utils::toUpper(_logfile) == "NONE") return (0);
		if (_logfile_stream.is_open()) _logfile_stream.close();

		_logfile_stream.open(_logfile, std::ios::app);
		if (!_logfile_stream.is_open()) {
			error("cannot open log file: " + _logfile + " - " + strerror(errno));
			return (1);
		}
		
		_logfile_stream << std::unitbuf;
		return (0);
	}

#pragma endregion

#pragma region "Getters"

	#pragma region "Logfile"

		std::string TaskmasterLog::get_logfile() const {
			return (_logfile);
		}

	#pragma endregion

	#pragma region "Max Bytes"

		long TaskmasterLog::get_logfile_maxbytes() const {
			return (_logfile_maxbytes);
		}

	#pragma endregion

	#pragma region "Backups"

		uint16_t TaskmasterLog::get_logfile_backups() const {
			return (_logfile_backups);
		}

	#pragma endregion

	#pragma region "Level"

		uint8_t TaskmasterLog::get_logfile_level() const {
			return (_logfile_backups);
		}

	#pragma endregion

	#pragma region "Syslog"

		bool TaskmasterLog::get_logfile_syslog() const {
			return (_logfile_syslog);
		}

	#pragma endregion

	#pragma region "Stdout"

		bool TaskmasterLog::get_logfile_stdout() const {
			return (_logfile_stdout);
		}

	#pragma endregion

	#pragma region "Ready"

		bool TaskmasterLog::get_logfile_ready() const {
			return (_logfile_ready);
		}

	#pragma endregion

#pragma endregion

#pragma region "Setters"

	#pragma region "Logfile"

		void TaskmasterLog::set_logfile(const std::string& logfile) {
			if (_logfile == logfile) return;
			_logfile = logfile;
			open();
		}

	#pragma endregion

	#pragma region "Max Bytes"

		void TaskmasterLog::set_logfile_maxbytes(std::string logfile_maxbytes) {
			_logfile_maxbytes = Utils::parse_size(logfile_maxbytes);
		}

		void TaskmasterLog::set_logfile_maxbytes(long logfile_maxbytes) {
			_logfile_maxbytes = logfile_maxbytes;
		}

	#pragma endregion

	#pragma region "Backups"

		void TaskmasterLog::set_logfile_backups(std::string logfile_backups) {
			_logfile_backups = std::atoi(logfile_backups.c_str());
		}

		void TaskmasterLog::set_logfile_backups(uint16_t logfile_backups) {
			_logfile_backups = logfile_backups;
		}

	#pragma endregion

	#pragma region "Level"

		void TaskmasterLog::set_logfile_level(std::string logfile_level) {
			_logfile_level = Utils::parse_loglevel(logfile_level);
		}

		void TaskmasterLog::set_logfile_level(uint8_t logfile_level) {
			_logfile_level = logfile_level;
		}

	#pragma endregion

	#pragma region "Syslog"

		void TaskmasterLog::set_logfile_syslog(std::string logfile_syslog) {
			_logfile_syslog = Utils::parse_bool(logfile_syslog);
		}

		void TaskmasterLog::set_logfile_syslog(bool logfile_syslog) {
			_logfile_syslog = logfile_syslog;
		}

	#pragma endregion

	#pragma region "Stdout"

		void TaskmasterLog::set_logfile_stdout(std::string logfile_stdout) {
			_logfile_stdout = Utils::parse_bool(logfile_stdout);
		}

		void TaskmasterLog::set_logfile_stdout(bool logfile_stdout) {
			_logfile_stdout = logfile_stdout;
		}

	#pragma endregion

	#pragma region "Ready"

		void TaskmasterLog::set_logfile_ready(std::string logfile_ready) {
			set_logfile_ready(Utils::parse_bool(logfile_ready));
		}

		void TaskmasterLog::set_logfile_ready(bool logfile_ready) {
			_logfile_ready = logfile_ready;

			if (_logfile_ready == true && !_logfile_buffer.empty()) {
				std::vector<std::string> levels = { "DEBU", "INFO", "WARN", "ERRO", "CRIT", "[GENERIC]" };
				levels.erase(levels.begin(), levels.begin() + _logfile_level);

				for (auto& log : _logfile_buffer) {
					bool ignore = true;

					for (const auto& level : levels) {
						if (log.find(level) != std::string::npos) { ignore = false; break; }
					}

					if (!ignore) {
						if (log.substr(0, 9) == "[GENERIC]") log = log.substr(9);
						if (_logfile_stream.is_open())	_logfile_stream << log;
						if (_logfile_stdout)			std::cout << log;
					}
				}

				_logfile_buffer.clear();
			}
		}

	#pragma endregion

#pragma endregion

#pragma region "Logging"

	#pragma region "Log"

		void TaskmasterLog::log(const std::string& msg, const std::string& level, bool add_level) {
			std::string log;

			if (add_level)	log = getTimestamp() + " " + level.substr(0, 4) + " " + msg + "\n";
			else			log = ((_logfile_ready) ? "" : "[GENERIC]") + msg + "\n";

			if (!_logfile_ready) { _logfile_buffer.push_back(log); return ; }

			if (_logfile_stream.is_open())	_logfile_stream << log;
			if (_logfile_stdout)			std::cout << log;
		}

	#pragma endregion

	#pragma region "GENERIC"

		void TaskmasterLog::generic(const std::string& msg) {
			if (_logfile_level > CRITICAL) return;
			log(msg, "GENERIC", false);
		}

	#pragma endregion

	#pragma region "DEBUG"

		void TaskmasterLog::debug(const std::string& msg) {
			if (_logfile_level > DEBUG) return;
			log(msg, "DEBUG");
		}

	#pragma endregion

	#pragma region "INFO"

		void TaskmasterLog::info(const std::string& msg) {
			if (_logfile_level > INFO) return;
			log(msg, "INFO");
		}

	#pragma endregion

	#pragma region "WARNING"

		void TaskmasterLog::warning(const std::string& msg) {
			if (_logfile_level > WARNING) return;
			log(msg, "WARNING");
		}

	#pragma endregion

	#pragma region "ERROR"

		void TaskmasterLog::error(const std::string& msg) {
			if (_logfile_level > ERROR) return;
			log(msg, "ERROR");
		}

	#pragma endregion

	#pragma region "CRITICAL"

		void TaskmasterLog::critical(const std::string& msg) {
			if (_logfile_level > CRITICAL) return;
			log(msg, "CRITICAL");
		}

	#pragma endregion

#pragma endregion

#pragma region "Get Time Stamp"

	std::string TaskmasterLog::getTimestamp() const {
		auto		now = std::chrono::system_clock::now();
		auto		time_t = std::chrono::system_clock::to_time_t(now);
		auto		ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
		struct tm	*timeinfo = localtime(&time_t);
		char		buffer[24], ms_buffer[4];

		strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
		sprintf(ms_buffer, "%03d", (int)ms.count());

		return (std::string(buffer) + "," + ms_buffer);
	}

#pragma endregion
			
// 0. Parsear opciones y mostrar errores en stderr
// 1. Crear clase vacia
// 2. Si logfile_ready = false, log todo a logfile_buffer
// 3. Al terminar la carga de configuracion, crear archivo si se debe
// 4. Guardar log del level (ignorar los que no son del level) en archivo
// 5. Si nodaemon y !silent imprimir log del level (ignorar los que no son del level) en stdout
// 6. logfile_ready = true... ya se procesan los logs normalmente
