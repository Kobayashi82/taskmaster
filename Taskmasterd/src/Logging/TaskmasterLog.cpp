/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TaskmasterLog.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/11 22:28:53 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/09 17:30:23 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Utils/Utils.hpp"
	#include "Config/Config.hpp"
	#include "Logging/TaskmasterLog.hpp"

	#include <cstring>															// strerror()
	#include <iostream>															// std::cout
	#include <chrono>															// std::chrono
	#include <syslog.h>															// syslog functions

#pragma endregion

#pragma region "Variables"

	TaskmasterLog Log;

#pragma endregion

#pragma region "Constructors"

	TaskmasterLog::TaskmasterLog() :
		_logfile(""), _logfile_maxbytes(0), _logfile_backups(0), _logfile_level(INFO),
		_logfile_syslog(false), _logfile_stdout(false), _logfile_ready(false),
		_buffer_max_size(255)
	{
		_logRotate.set_ReopenCallback([this]() {
			if (!_logfile.empty()) open();
		});
	}

	TaskmasterLog::TaskmasterLog(const std::string& logFile, size_t maxSize, uint16_t maxBackups, uint8_t level, bool syslog, bool _stdout, bool ready) :
		_logfile(logFile), _logfile_maxbytes(maxSize), _logfile_backups(maxBackups), _logfile_level(level),
		_logfile_syslog(syslog), _logfile_stdout(_stdout), _logfile_ready(ready),
		_buffer_max_size(255)
	{
		_logRotate.set_ReopenCallback([this]() {
			if (!_logfile.empty()) open();
		});
	}

	TaskmasterLog::~TaskmasterLog() {
		if (_logfile_stream.is_open()) _logfile_stream.close();
		if (_logfile_syslog) closelog();
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

	#pragma region "Buffer"

		size_t TaskmasterLog::get_buffer_size() const {
			return (_logfile_buffer.size());
		}

		size_t TaskmasterLog::get_buffer_max_size() const {
			return (_buffer_max_size);
		}
	
		std::deque<std::string> TaskmasterLog::get_log(size_t n) const {
			if (n == 0) return {};

			if (n >= _logfile_buffer.size()) return (_logfile_buffer);
			return (std::deque<std::string>(_logfile_buffer.end() - n, _logfile_buffer.end()));
		}

	#pragma endregion

#pragma endregion

#pragma region "Setters"

	#pragma region "Logfile"

		void TaskmasterLog::set_logfile(const std::string& logfile) {
			if (_logfile == logfile) return;
			_logfile = logfile;
			_logRotate.set_LogPath(logfile);
			open();
		}

	#pragma endregion

	#pragma region "Max Bytes"

		void TaskmasterLog::set_logfile_maxbytes(long logfile_maxbytes) {
			_logfile_maxbytes = logfile_maxbytes;
			_logRotate.set_MaxSize(static_cast<size_t>(logfile_maxbytes));
		}

	#pragma endregion

	#pragma region "Backups"

		void TaskmasterLog::set_logfile_backups(uint16_t logfile_backups) {
			_logfile_backups = logfile_backups;
			_logRotate.set_MaxBackups(logfile_backups);
		}

	#pragma endregion

	#pragma region "Level"

		void TaskmasterLog::set_logfile_level(uint8_t logfile_level) {
			_logfile_level = logfile_level;
		}

	#pragma endregion

	#pragma region "Syslog"

		void TaskmasterLog::set_logfile_syslog(bool logfile_syslog) {
			_logfile_syslog = logfile_syslog;
		}

	#pragma endregion

	#pragma region "Stdout"

		void TaskmasterLog::set_logfile_stdout(bool logfile_stdout) {
			_logfile_stdout = logfile_stdout;
		}

	#pragma endregion

	#pragma region "Ready"

		void TaskmasterLog::set_logfile_ready(bool logfile_ready) {
			if (_logfile_ready == logfile_ready) return;

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
						std::string output_log = log;
						if (output_log.substr(0, 9) == "[GENERIC]") output_log = output_log.substr(9);
						if (_logfile_stream.is_open())	_logfile_stream << output_log;
						if (_logfile_stdout)			std::cout << output_log;
						if (_logfile_syslog ) {
							std::string level;
							if (output_log.find("DEBU") != std::string::npos) level = "DEBUG";
							if (output_log.find("INFO") != std::string::npos) level = "INFO";
							if (output_log.find("WARN") != std::string::npos) level = "WARNING";
							if (output_log.find("ERRO") != std::string::npos) level = "ERROR";
							if (output_log.find("CRIT") != std::string::npos) level = "CRITICAL";

							if (!level.empty()) send_syslog(output_log.substr(29), level);
						}
					}
				}
			}
		}

	#pragma endregion

	#pragma region "Buffer"

		void TaskmasterLog::set_buffer_max_size(size_t size) {
			_buffer_max_size = size;
		}

	#pragma endregion

#pragma endregion

#pragma region "Manage"

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

	#pragma region "Close"

		void TaskmasterLog::close() {
			if (_logfile_stream.is_open()) _logfile_stream.close();
			if (_logfile_syslog) closelog();
		}

	#pragma endregion

#pragma endregion

#pragma region "Logging"

	#pragma region "Syslog"

		void TaskmasterLog::send_syslog(const std::string& msg, const std::string& level) {
			static bool syslog_opened = false;

			if (!syslog_opened) {
				openlog("taskmasterd", LOG_PID | LOG_CONS, LOG_DAEMON);
				syslog_opened = true;
			}

			int priority = LOG_INFO;
			if (level == "DEBUG")		priority = LOG_DEBUG;
			if (level == "INFO")		priority = LOG_INFO;
			if (level == "WARNING")		priority = LOG_WARNING;
			if (level == "ERROR")		priority = LOG_ERR;
			if (level == "CRITICAL")	priority = LOG_CRIT;

			syslog(priority, "%s", msg.c_str());
		}

	#pragma endregion

	#pragma region "Get Time Stamp"

		std::string TaskmasterLog::get_timestamp() const {
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

	#pragma region "Add To Buffer"

		void TaskmasterLog::add_buffer(const std::string& log) {
			if (log.substr(0, 9) == "[GENERIC]")	_logfile_buffer.push_back(log.substr(9));
			else									_logfile_buffer.push_back(log);

			if (_logfile_buffer.size() > _buffer_max_size) _logfile_buffer.pop_front();
		}

	#pragma endregion

	#pragma region "Log"

		void TaskmasterLog::log(const std::string& msg, const std::string& level, bool add_level) {
			std::string log;

			if (add_level)	log = get_timestamp() + " " + level.substr(0, 4) + " " + msg + "\n";
			else			log = ((_logfile_ready) ? "" : "[GENERIC]") + msg + "\n";

			add_buffer(log);

			if (_logfile_ready) {
				if (log.substr(0, 9) == "[GENERIC]") log = log.substr(9);

				if (_logfile_stream.is_open()) {
					_logfile_stream << log;
					_logfile_stream.flush();
					_logRotate.rotate(_logfile);
				}
				if (_logfile_stdout) std::cout << log;
				if (_logfile_syslog && add_level) send_syslog(log.substr(29), level);
			}
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
