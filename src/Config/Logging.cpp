/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logging.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/11 22:28:53 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/18 22:20:16 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Options.hpp"
	#include "Config/Logging.hpp"

	#include <cstring>															// strerror()
	#include <filesystem>														// path(), absolute(), exists(), create_directories()

#pragma endregion

#pragma region "Variables"

	Tintin_reporter	*Log;														// Global pointer to Tintin_reporter

#pragma endregion

#pragma region "Tintin Reporter"

	#pragma region "Constructors"

		#pragma region "Default"

			Tintin_reporter::Tintin_reporter(const std::string& logPath, uint8_t logLevel) : _logPath(logPath), _logLevel(logLevel) {
				if (!std::filesystem::path(_logPath).is_absolute()) _logPath = std::filesystem::absolute(_logPath).string();
				createDirectory(_logPath);

				if (Options::logNew)	_logFile.open(_logPath, std::ios::trunc);
				else					_logFile.open(_logPath, std::ios::app);

				if (!_logFile.is_open()) {
					std::string errorMsg = "Cannot open log file: " + _logPath + " - " + strerror(errno);
					throw std::runtime_error(errorMsg);
				}

				_logFile << std::unitbuf;
				Log = this;
			}

		#pragma endregion

		#pragma region "Copy"

			Tintin_reporter::Tintin_reporter(const Tintin_reporter& src) : _logPath(src._logPath), _logLevel(src._logLevel) {
				if (!std::filesystem::path(_logPath).is_absolute()) _logPath = std::filesystem::absolute(_logPath).string();
				createDirectory(_logPath);

				if (Options::logNew)	_logFile.open(_logPath, std::ios::trunc);
				else					_logFile.open(_logPath, std::ios::app);

				if (!_logFile.is_open()) {
					std::string errorMsg = "Cannot open log file: " + _logPath + " - " + strerror(errno);
					throw std::runtime_error(errorMsg);
				}

				_logFile << std::unitbuf;
			}

		#pragma endregion

		#pragma region "Destructror"

			Tintin_reporter::~Tintin_reporter() {
				if (_logFile.is_open()) _logFile.close();
			}

		#pragma endregion

	#pragma endregion

	#pragma region "Overloads"

		#pragma region "Copy"

			Tintin_reporter& Tintin_reporter::operator=(const Tintin_reporter& rhs) {
				if (this != &rhs) {
					if (_logFile.is_open()) _logFile.close();

					_logPath = rhs._logPath;
					_logLevel = rhs._logLevel;
					createDirectory(_logPath);
					_logFile.open(_logPath, std::ios::app);
					if (!_logFile.is_open()) {
						std::string errorMsg = "Cannot open log file: " + _logPath + " - " + strerror(errno);
						throw std::runtime_error(errorMsg);
					}

					_logFile << std::unitbuf;
				}

				return (*this);
			}

		#pragma endregion

		#pragma region "Equality"

			bool Tintin_reporter::operator==(const Tintin_reporter& rhs) const {
				return (_logPath == rhs._logPath && _logLevel == rhs._logLevel);
			}

		#pragma endregion

	#pragma endregion

	#pragma region "Setter & Getters"
	
		void Tintin_reporter::set_logLevel(uint8_t logLevel) { _logLevel = logLevel; }
		uint8_t Tintin_reporter::get_logLevel() const { return (_logLevel); }
		std::string Tintin_reporter::get_logPath() const { return (_logPath); }

	#pragma endregion

	#pragma region "Open"

		void Tintin_reporter::open() {
			if (_logFile.is_open()) _logFile.close();

			createDirectory(_logPath);
			_logFile.open(_logPath, std::ios::app);
			if (!_logFile.is_open()) {
				std::string errorMsg = "Cannot open log file: " + _logPath + " - " + strerror(errno);
				throw std::runtime_error(errorMsg);
			}

			_logFile << std::unitbuf;
		}

	#pragma endregion

	#pragma region "Clear"

		void Tintin_reporter::clear() {
			if (_logFile.is_open()) _logFile.close();

			createDirectory(_logPath);
			_logFile.open(_logPath, std::ios::trunc);
			if (!_logFile.is_open()) {
				std::string errorMsg = "Cannot open log file: " + _logPath + " - " + strerror(errno);
				throw std::runtime_error(errorMsg);
			}

			_logFile << std::unitbuf;
		}

	#pragma endregion

	#pragma region "Logging"

		void Tintin_reporter::debug(const std::string& msg) {
			if (_logLevel > DEBUG) return;
			std::lock_guard<std::mutex> lock(_mutex);
			try {
				if (!std::filesystem::exists(_logPath) || !_logFile.is_open()) open();
			} catch (const std::filesystem::filesystem_error&) {
				if (!_logFile.is_open()) open();
			}
			rotateLog();
			_logFile << "[" << getTimestamp() << "]      [ DEBUG ] - " << msg << std::endl;
		}

		void Tintin_reporter::info(const std::string& msg) {
			if (_logLevel > INFO) return;
			std::lock_guard<std::mutex> lock(_mutex);
			try {
				if (!std::filesystem::exists(_logPath) || !_logFile.is_open()) open();
			} catch (const std::filesystem::filesystem_error&) {
				if (!_logFile.is_open()) open();
			}
			rotateLog();
			_logFile << "[" << getTimestamp() << "]       [ INFO ] - " << msg << std::endl;
		}

		void Tintin_reporter::log(const std::string& msg) {
			if (_logLevel > LOG) return;
			std::lock_guard<std::mutex> lock(_mutex);
			try {
				if (!std::filesystem::exists(_logPath) || !_logFile.is_open()) open();
			} catch (const std::filesystem::filesystem_error&) {
				if (!_logFile.is_open()) open();
			}
			rotateLog();
			_logFile << "[" << getTimestamp() << "]        [ LOG ] - " << msg << std::endl;
		}

		void Tintin_reporter::warning(const std::string& msg) {
			if (_logLevel > WARNING) return;
			std::lock_guard<std::mutex> lock(_mutex);
			try {
				if (!std::filesystem::exists(_logPath) || !_logFile.is_open()) open();
			} catch (const std::filesystem::filesystem_error&) {
				if (!_logFile.is_open()) open();
			}
			rotateLog();
			_logFile << "[" << getTimestamp() << "]    [ WARNING ] - " << msg << std::endl;
		}

		void Tintin_reporter::error(const std::string& msg) {
			if (_logLevel > ERROR) return;
			std::lock_guard<std::mutex> lock(_mutex);
			try {
				if (!std::filesystem::exists(_logPath) || !_logFile.is_open()) open();
			} catch (const std::filesystem::filesystem_error&) {
				if (!_logFile.is_open()) open();
			}
			rotateLog();
			_logFile << "[" << getTimestamp() << "]      [ ERROR ] - " << msg << std::endl;
		}

		void Tintin_reporter::critical(const std::string& msg) {
			if (_logLevel > CRITICAL) return;
			std::lock_guard<std::mutex> lock(_mutex);
			try {
				if (!std::filesystem::exists(_logPath) || !_logFile.is_open()) open();
			} catch (const std::filesystem::filesystem_error&) {
				if (!_logFile.is_open()) open();
			}
			rotateLog();
			_logFile << "[" << getTimestamp() << "]   [ CRITICAL ] - " << msg << std::endl;
		}

	#pragma endregion

	#pragma region "Rotate Log"

		void Tintin_reporter::rotateLog() {
			if (!Options::logSize || !_logFile.is_open()) return;

			std::streampos currentPos = _logFile.tellp();

			if ((size_t)currentPos > Options::logSize) {
				_logFile.close();
				if (!Options::logMax) clear();
				else {
					std::string oldestFile = _logPath + "." + std::to_string(Options::logMax);
					std::remove(oldestFile.c_str());

					for (int i = Options::logMax - 1; i >= 1; i--) {
						std::string currentFile = _logPath + "." + std::to_string(i);
						std::string nextFile = _logPath + "." + std::to_string(i + 1);

						std::rename(currentFile.c_str(), nextFile.c_str());
					}

					std::string firstBackup = _logPath + ".1";
					std::rename(_logPath.c_str(), firstBackup.c_str());

					open();
				}
			}
		}

	#pragma endregion

	#pragma region "Utils"

		#pragma region "Get Time Stamp"

			std::string Tintin_reporter::getTimestamp() {
				time_t		now = time(0);
				struct tm*	timeinfo = localtime(&now);

				char buffer[20];
				strftime(buffer, sizeof(buffer), "%d/%m/%Y-%H:%M:%S", timeinfo);

				return (std::string(buffer));
			}

		#pragma endregion

		#pragma region "Create Directory"

			void Tintin_reporter::createDirectory(const std::string& filePath) {
				try {
					std::filesystem::path p(filePath);
					auto parent = p.parent_path();
					if (!parent.empty() && !std::filesystem::exists(parent)) {
						std::filesystem::create_directories(parent);
					}
				} catch (const std::filesystem::filesystem_error& e) {}
			}

		#pragma endregion

	#pragma endregion

#pragma endregion
