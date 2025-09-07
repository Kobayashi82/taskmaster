/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LogRotate.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/11 22:28:53 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/07 18:19:18 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Logging/TaskmasterLog.hpp"
	#include "Logging/LogRotate.hpp"

	#include <iostream>																// std::cerr
	#include <filesystem>															// std::filesystem
	#include <cstdio>																// std::remove, std::rename

#pragma endregion

#pragma region "Constructors"

	LogRotate::LogRotate() :
		_logPath(""), _maxSize(0), _maxBackups(0), enabled(true)
	{}

	LogRotate::LogRotate(const std::string& logPath, size_t maxSize, uint16_t maxBackups) :
		_logPath(logPath), _maxSize(maxSize), _maxBackups(maxBackups), enabled(true)
	{}

#pragma endregion

#pragma region "Getters"

	std::string LogRotate::get_LogPath() const {
		return (_logPath);
	}

	size_t LogRotate::get_MaxSize() const {
		return (_maxSize);
	}

	uint16_t LogRotate::get_MaxBackups() const {
		return (_maxBackups);
	}

#pragma endregion

#pragma region "Setters"

	void LogRotate::set_LogPath(const std::string& logPath) {
		_logPath = logPath;
	}

	void LogRotate::set_MaxSize(size_t maxSize) {
		_maxSize = maxSize;
	}

	void LogRotate::set_MaxBackups(uint16_t maxBackups) {
		_maxBackups = maxBackups;
	}

	void LogRotate::set_ReopenCallback(std::function<void()> callback) {
		_reopenCallback = callback;
	}

#pragma endregion

#pragma region "Clear Log"

void LogRotate::clear_log() {
	try {
		if (std::remove(_logPath.c_str()) != 0) {
			enabled = false;
			Log.error("LogRotate: Failed to clear log file " + _logPath + ". Rotation disabled");
		}
	} catch (const std::exception& ex) {
		enabled = false;
		Log.error("LogRotate: Failed to clear log: " + std::string(ex.what()) + ". Rotation disabled");
	}
}

#pragma endregion

#pragma region "Rotate Files"

void LogRotate::rotate_files() {
	if (_maxBackups == 0) { clear_log(); return; }

	try {
		std::string oldestFile = _logPath + "." + std::to_string(_maxBackups);
		std::remove(oldestFile.c_str());

		for (int i = _maxBackups - 1; i >= 1; --i) {
			std::string currentFile	= _logPath + "." + std::to_string(i);
			std::string nextFile	= _logPath + "." + std::to_string(i + 1);

			if (std::filesystem::exists(currentFile)) {
				if (std::rename(currentFile.c_str(), nextFile.c_str()) != 0) {
					enabled = false;
					Log.error("LogRotate: Failed to rename " + currentFile + " to " + nextFile  + ". Rotation disabled");
					return;
				}
			}
		}

		std::string firstBackup = _logPath + ".1";
		if (std::rename(_logPath.c_str(), firstBackup.c_str()) != 0) {
			enabled = false;
			Log.error("LogRotate: Failed to rename " + _logPath + " to " + firstBackup + ". Rotation disabled");
			return;
		}
	} catch (const std::exception& ex) {
		enabled = false;
		Log.error("LogRotate: failed - " + std::string(ex.what()) + ". Rotation disabled");
	}
}

#pragma endregion

#pragma region "Should Rotate"

bool LogRotate::should_rotate(const std::string& filePath) {
	if (!enabled || !_maxSize || filePath.empty()) return (false);

	try {
		if (!std::filesystem::exists(filePath)) return (false);
		
		auto st = std::filesystem::status(filePath);
		if (std::filesystem::is_character_file(st) || 
			std::filesystem::is_block_file(st) || 
			std::filesystem::is_fifo(st))
		{
			enabled = false;
			return (false);
		}
		
		size_t fileSize = std::filesystem::file_size(filePath);
		return (fileSize > _maxSize);
	} catch (const std::filesystem::filesystem_error& ex) { return (false); }
}

#pragma endregion

#pragma region "Rotate"

void LogRotate::rotate(const std::string& filePath) {
	if (!_maxSize || filePath.empty()) return;

	if (should_rotate(filePath)) {
		if (_logPath.empty() || _logPath != filePath) _logPath = filePath;
		
		rotate_files();
		
		if (_reopenCallback) _reopenCallback();
	}
}

#pragma endregion
