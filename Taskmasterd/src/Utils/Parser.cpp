/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/03 21:12:54 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/05 13:21:39 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Utils/Utils.hpp"
	#include "Config/Config.hpp"

	#include <unistd.h>															// getuid()
	#include <sys/resource.h>													// getrlimit(), setrlimit()
	#include <sstream>															// std::istringstream, std::getline()
	#include <iostream>

#pragma endregion

#pragma region "Remove Quotes"

	std::string Utils::remove_quotes(const std::string& str) {
		std::string	result;
		char		quoteChar = 0;
		bool		escaped = false;

		for (char c : str) {
			if (escaped)								{ escaped = false;	result += c;	continue; }
			if (!quoteChar && c == '\\')				{ escaped = true;					continue; }
			if (!quoteChar && (c == '"' || c == '\''))	{ quoteChar = c;					continue; }
			if (quoteChar && c == quoteChar)			{ quoteChar = 0;					continue; }

			result += c;
		}

		if (quoteChar || escaped) throw std::runtime_error("unclosed quote or unfinished escape sequence");

		return (result);
	}

#pragma endregion

#pragma region "Comments"

	std::string Utils::remove_comments(const std::string& line) {
		char	quoteChar = 0;
		bool	escaped = false;

		for (size_t i = 0; i < line.length(); ++i) {
			char c = line[i];

			if (escaped)								{ escaped = false;	continue; }
			if (!quoteChar && c == '\\')				{ escaped = true;	continue; }
			if (!quoteChar && (c == '"' || c == '\''))	{ quoteChar = c;	continue; }
			if (quoteChar && c == quoteChar)			{ quoteChar = 0;	continue; }

			if (!quoteChar && (c == '#' || c == ';'))	return (line.substr(0, i));
		}

		return (line);
	}

#pragma endregion

#pragma region "FD Limit"

	int Utils::parse_fd_limit(uint16_t minfds) {
		struct rlimit rl;

		if (!getrlimit(RLIMIT_NOFILE, &rl)) {
			if (rl.rlim_cur > minfds) return (0);
			else if (getuid() == 0 && rl.rlim_cur < rl.rlim_max && minfds <= rl.rlim_max) {
				rl.rlim_cur = minfds;
				return (setrlimit(RLIMIT_NOFILE, &rl) != 0);
			}
		}

		return (1);
	}

#pragma endregion

#pragma region "Process Limit"

	int Utils::parse_process_limit(uint16_t minprocs) {
		struct rlimit rl;

		if (getrlimit(RLIMIT_NPROC, &rl) == 0) {
			if (rl.rlim_cur > minprocs) return (0);
			else if (getuid() == 0 && rl.rlim_cur < rl.rlim_max && minprocs <= rl.rlim_max) {
				rl.rlim_cur = minprocs;
				return (setrlimit(RLIMIT_NPROC, &rl) != 0);
			}
		}

		return (1);
	}

#pragma endregion

#pragma region "Size"

	long Utils::parse_size(const std::string &value) {
		if (value.empty()) return (0);

		char *end;
		long num = std::strtol(value.c_str(), &end, 10);
		if (errno == ERANGE)		return (-1);

		std::string suffix = Utils::trim(Utils::toUpper(std::string(end)));

		if		(suffix.empty())	return (num);
		else if	(suffix == "BYTES")	return (num);
		else if	(suffix == "B" )	return (num);
		else if (suffix == "KB")	return (num * 1024);
		else if (suffix == "MB")	return (num * 1024 * 1024);
		else if (suffix == "GB")	return (num * 1024 * 1024 * 1024);
		else						return (-1);
	}

#pragma endregion

#pragma region "Number"

	long Utils::parse_number(const std::string& value, long min, long max, long default_value) {
		if (value.empty()) return (default_value);

		char	*end;
		long	num = std::strtol(value.c_str(), &end, 10);

		if (errno == ERANGE)		return (default_value);
		if (*end != '\0')			return (default_value);
		if (num < min || num > max)	return (default_value);

		return (num);
	}

#pragma endregion

#pragma region "Signal"

	uint8_t Utils::parse_signal(const std::string& value) {
		if (value.empty()) return (15);

		static const std::map<std::string, int> signals = {
			{"HUP", 1}, {"SIGHUP", 1},
			{"INT", 2}, {"SIGINT", 2},
			{"QUIT", 3}, {"SIGQUIT", 3},
			{"KILL", 9}, {"SIGKILL", 9},
			{"TERM", 15}, {"SIGTERM", 15},
			{"USR1", 10}, {"SIGUSR1", 10},
			{"USR2", 12}, {"SIGUSR2", 12}
		};

		std::string signal = Utils::toUpper(value);
		if (Utils::isDigit(signal)) return (std::stoi(signal));

		auto it = signals.find(signal);
		if (it != signals.end()) return (it->second);

		return (15);
	}

#pragma endregion

#pragma region "Bool"

	int Utils::parse_bool(const std::string &value, bool unexpected) {
		if (value.empty()) return (0);

		std::string val = Utils::toLower(value);
		if (unexpected && val == "unexpected") return (2);

		return (val == "true" || val == "1" || val == "yes");
	}

#pragma endregion

#pragma region "LogLevel"

	uint8_t Utils::parse_loglevel(const std::string &value) {
		if (value.empty()) return (1);

		std::string level = Utils::toUpper(value);

		if (level == "0" || level == "DEBUG")						return (0);
		if (level == "1" || level == "INFO")						return (1);
		if (level == "2" || level == "WARNING" || level == "WARN")	return (2);
		if (level == "3" || level == "ERROR")						return (3);
		if (level == "4" || level == "CRITICAL")					return (4);

		return (1);
	}

#pragma endregion

#pragma region "Executable"

	std::string Utils::parse_executable(const std::string& value) {
		if (value.empty()) return {};
		std::string val = trim(value);

		std::string	command;
		char		quoteChar = 0;
		bool		escaped = false;

		for (char c : val) {
			if (escaped)									{ escaped = false;	command += c;	continue; }
			if (!quoteChar && c=='\\')						{ escaped = true;					continue; }
			if (!quoteChar && (c == '"' || c == '\''))		{ quoteChar = c;					continue; }
			if (quoteChar && c == quoteChar)				{ quoteChar = 0;					continue; }
			if (!quoteChar && isspace((unsigned char)c))										break;

			command += c;
		}

		if (quoteChar || escaped) return {};

		auto check_exec = [](const std::string& path) -> bool { return (!access(path.c_str(), X_OK)); };
		if (command.find('/') != std::string::npos) return (check_exec(command) ? (command) : "");

		const char *path_env = std::getenv("PATH");
		std::string path = path_env ? path_env : "";
		if (!path.empty()) {
			std::istringstream ss(path); std::string dir;
			while (std::getline(ss, dir, ':')) {
				if (dir.empty()) dir = ".";
				if (dir.back() == '/') dir.pop_back();
				std::string candidate = dir + "/" + command;
				if (check_exec(candidate)) return (candidate);
			}
		}

		return {};
	}

#pragma endregion
