/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 11:34:14 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/31 00:05:24 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Parser.hpp"

	#include <unistd.h>															// getuid()
	#include <algorithm>														// std::transform()
	#include <filesystem>														// std::filesystem::path(), std::filesystem::parent_path(), std::filesystem::current_path(), std::filesystem::temp_directory_path(), std::filesystem::weakly_canonical(), std::filesystem::exists()
	#include <sys/resource.h>													// getrlimit(), setrlimit()
	#include <pwd.h>															// struct passwd, getpwnam()

#pragma endregion

#pragma region "Trim"

	std::string ConfigParser::trim(const std::string& str) const {
		size_t first = str.find_first_not_of(" \t\r\n");
		if (first == std::string::npos) return ("");

		size_t last = str.find_last_not_of(" \t\r\n");
		return (str.substr(first, (last - first + 1)));
	}

#pragma endregion

#pragma region "To Upper"

	std::string ConfigParser::toUpper(const std::string& str) const {
		std::string result = str;
		std::transform(result.begin(), result.end(), result.begin(), ::toupper);

		return (result);
	}

#pragma endregion

#pragma region "To Lower"

	std::string ConfigParser::toLower(const std::string& str) const {
		std::string result = str;
		std::transform(result.begin(), result.end(), result.begin(), ::tolower);

		return (result);
	}

#pragma endregion

#pragma region "Expand Path"

	std::string ConfigParser::expand_path(const std::string& path, const std::string current_path, bool expand_symbolic, bool weakly) const {
		if (path.empty()) return ("");

		std::filesystem::path p;
		std::filesystem::path cp = (current_path.empty()) ? std::filesystem::current_path() : std::filesystem::path(current_path);

		// is home
		if (path[0] == '~') {
			const char *home = std::getenv("HOME");
			if (!home) {
				struct passwd *pw = getpwuid(getuid());
				if (pw) home = pw->pw_dir;
			}
			if (!home) return ("");
			p = std::filesystem::path(home) / path.substr(1);
		}
		// is absolute
		else if (std::filesystem::path(path).is_absolute()) p = path;
		// is relative
		else p = cp / path;

		// resolve symbolic links
		if (expand_symbolic) {
			if (weakly) {
				try { return (std::filesystem::weakly_canonical(p).string()); }
				catch (const std::filesystem::filesystem_error&) { return (""); }
			} else {
				try { return (std::filesystem::canonical(p).string()); }
				catch (const std::filesystem::filesystem_error&) { return (""); }
			}
		}

		return (p);
	}

#pragma endregion

#pragma region "Temp Path"

	std::string ConfigParser::temp_path() const {
		try { return (std::filesystem::temp_directory_path().string()); }
		catch (const std::filesystem::filesystem_error& e) { return (""); }
	}

#pragma endregion

#pragma region "Config Path"

	std::string ConfigParser::config_path() const {
		const std::string candidates[] = {
			"../etc/supervisord.conf",
			"../supervisord.conf",
			"supervisord.conf",
			"etc/supervisord.conf",
			"/etc/supervisord.conf",
			"/etc/supervisor/supervisord.conf"
		};

		for (const auto& path : candidates) {
			std::string expanded = expand_path(path);
			if (!expanded.empty() && std::filesystem::exists(expanded)) return (expanded);
		}

		return ("");
	}

#pragma endregion

#pragma region "Check FD Limit"

	int ConfigParser::check_fd_limit(uint16_t minfds) const {
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

#pragma region "Check Process Limit"

	int ConfigParser::check_process_limit(uint16_t minprocs) const {
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

#pragma region "Parse Size"

	long ConfigParser::parse_size(const std::string &value) const {
		char *end;
		long num = std::strtol(value.c_str(), &end, 10);

		std::string suffix(end);
		suffix = trim(suffix);
		for (auto &c : suffix) c = toupper(c);

		if		(suffix.empty())	return (num);
		else if	(suffix == "BYTES")	return (num);
		else if	(suffix == "B" )	return (num);
		else if (suffix == "KB")	return (num * 1024);
		else if (suffix == "MB")	return (num * 1024 * 1024);
		else if (suffix == "GB")	return (num * 1024 * 1024 * 1024);
		else						return (-1);
	}

#pragma endregion

#pragma region "Parse Files"

	std::vector<std::string> ConfigParser::parse_files(const std::string& fileString) {
		std::vector<std::string>	files;
		std::string					current;
		bool						inQuotes = false;
		bool						quotedToken = false;
		char						quoteChar = 0;

		auto pushToken = [&](bool wasQuoted) {
			if (!current.empty()) {
				std::string token = wasQuoted ? current : trim(current);
				if (!token.empty()) files.push_back(token);
				current.clear();
			}
		};

		for (size_t i = 0; i < fileString.size(); ++i) {
			char c = fileString[i];

			if		(c == '\\' && i + 1 < fileString.size())			  current.push_back(fileString[++i]);
			else if	((c == '"' || c == '\'') && !inQuotes)				{ inQuotes = quotedToken = true; quoteChar = c; }
			else if	(inQuotes && c == quoteChar)						  inQuotes = false;
			else if	(!inQuotes && (c == ' ' || c == ',' || c == '\t'))	{ pushToken(quotedToken); quotedToken = false; }
			else														  current.push_back(c);
		}

		pushToken(quotedToken);

		for (auto& file : files) {
			std::string fullpath = expand_path(file, configPath.parent_path(), false);
			if (!fullpath.empty()) file = fullpath;
		}

		files = globbing_expand(files);

 		return (files);
	}

#pragma endregion

#pragma region "Command Executable"

	bool ConfigParser::command_executable(const std::string& input, std::string& resolved) const {
		std::string	command;
		char		quoteChar = 0;
		bool		escaped = false;

		for (char c : input) {
			if (escaped)									{ command += c; escaped = false;	continue; }
			if (c=='\\')									{ escaped = true;					continue; }
			if (!quoteChar && (c == '"' || c == '\''))		{ quoteChar = c;					continue; }
			if (quoteChar && c == quoteChar)				{ quoteChar = 0;					continue; }
			if (!quoteChar && isspace((unsigned char)c))										break;

			command += c;
		}

		auto check_exec = [](const std::string& path) -> bool { return (!access(path.c_str(), X_OK)); };
		if (command.find('/') != std::string::npos) return (check_exec(command) ? (resolved=command, true) : false);

		std::string path = environment_get(environment, "PATH");
		if (!path.empty()) {
			std::istringstream ss(path); 
			std::string dir;
			while (std::getline(ss, dir, ':')) {
				std::string candidate = dir + "/" + command;
				if (check_exec(candidate)) { resolved = candidate; return (true); }
			}
		}

		return (false);
	}

#pragma endregion
