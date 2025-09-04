/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Error.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/04 12:03:15 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/04 12:13:20 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Utils/Utils.hpp"
	#include "Logging/TaskmasterLog.hpp"

	#include <algorithm>														// std::sort()

#pragma endregion

#pragma region "Variables"

	std::vector<Utils::ErrorInfo>	Utils::errors;
	uint16_t						Utils::errors_maxLevel = DEBUG;

#pragma endregion

#pragma region "Add"

	void Utils::error_add(std::string& filename, std::string msg, uint8_t level, uint16_t line, uint16_t order) {
		errors.push_back({filename, msg, level, line, order});
	}

#pragma endregion

#pragma region "Print"

	void Utils::error_print() {
		std::vector<std::string>						validLevels = { "DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL", "GENERAL" };
		std::map<std::string, std::vector<ErrorInfo>>	errorsByFile;
		std::vector<std::string>						fileOrder;

		std::sort(errors.begin(), errors.end(), [](const ErrorInfo& a, const ErrorInfo& b) { return (a.order < b.order); });

		for (const auto& error : errors) {
			if (errorsByFile.find(error.filename) == errorsByFile.end()) fileOrder.push_back(error.filename);
			errorsByFile[error.filename].push_back(error);
		}

		for (const std::string& filename : fileOrder) {
			std::string	all_errors;
			int			maxLevel = DEBUG;

			for (const auto& error : errorsByFile[filename]) {
				if (error.level > maxLevel) maxLevel = error.level;
				std::string line = (error.line) ? "in line " + std::to_string(error.line) : "\t\t";
				all_errors += "[" + validLevels[error.level].substr(0, 4) + "] " + line + "\t" + error.msg + "\n";
			}

			if (maxLevel > errors_maxLevel) errors_maxLevel = maxLevel;
			if (!all_errors.empty()) {
				all_errors.pop_back();
				switch (maxLevel) {
					case DEBUG:		Log.debug		(filename + "\n" + all_errors);	break;
					case INFO:		Log.info		(filename + "\n" + all_errors);	break;
					case WARNING:	Log.warning		(filename + "\n" + all_errors);	break;
					case ERROR:		Log.error		(filename + "\n" + all_errors);	break;
					case CRITICAL:	Log.critical	(filename + "\n" + all_errors);	break;
					case GENERIC:	Log.generic		(filename + "\n" + all_errors);	break;
					default:		Log.generic		(filename + "\n" + all_errors);	break;
				}
			}
		}
	}

#pragma endregion