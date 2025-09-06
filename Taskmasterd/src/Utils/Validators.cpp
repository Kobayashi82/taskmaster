/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Validators.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 11:32:25 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/06 21:44:37 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Utils/Utils.hpp"

	#include <unistd.h>															// access()
	#include <filesystem>														// std::filesystem::path(), std::filesystem::is_directory(), std::filesystem::exists()
	#include <pwd.h>															// struct passwd, getpwuid(), getpwnam()
	#include <grp.h>															// struct group, getgrnam()
	#include <regex>															// std::regex_match()
	#include <climits>															// INT_MAX
	#include <set>																// std::set

#pragma endregion

#pragma region "Bool"

	bool Utils::valid_bool(const std::string& value) {
		std::string lower = toLower(value);
		return (lower == "true" || lower == "false" || lower == "1" || lower == "0" || lower == "yes" || lower == "no");
	}

#pragma endregion

#pragma region "Number"

	bool Utils::valid_number(const std::string& value, long min, long max) {
		if (value.empty()) return (false);

		char	*end;
		long	num = std::strtol(value.c_str(), &end, 10);

		return (*end == '\0' && num >= min && num <= max);
	}

#pragma endregion

#pragma region "Path"

	bool Utils::valid_path(const std::string& value, const std::string current_path, bool is_directory, bool allow_auto, bool allow_none) {
		std::string fullPath;

		if (value.empty())											{ errno = EINVAL;	return (false); }
		if (allow_none && toLower(value) == "none")										return (true);
		if (allow_auto && toLower(value) == "auto")										return (true);
		else if (is_directory && toLower(value) == "do not change")	fullPath = expand_path(".", current_path);
		else if (is_directory)										fullPath = expand_path(value, current_path, true, false);
		else														fullPath = expand_path(value, current_path);

		if (fullPath.empty())										{ errno = ENOENT;	return (false); }
		std::filesystem::path p(fullPath);

		auto st = std::filesystem::status(p);
		if (std::filesystem::is_character_file(st))										return (true);
		if (std::filesystem::is_block_file(st))											return (true);
		if (std::filesystem::is_fifo(st))												return (true);

		if (is_directory) {
			if (!std::filesystem::exists(p))						{ errno = ENOENT;	return (false); }
			if (!std::filesystem::is_directory(p))					{ errno = ENOTDIR;	return (false); }
			if (access(p.c_str(), W_OK | X_OK))						{ errno = EACCES;	return (false); }

			return (true);
		}

		std::filesystem::path dir = p.parent_path();
		if (!std::filesystem::exists(dir))							{ errno = ENOENT;	return (false); }
		if (!std::filesystem::is_directory(dir))					{ errno = ENOTDIR;	return (false); }
		if (access(dir.c_str(), W_OK))								{ errno = EACCES;	return (false); }


		if (std::filesystem::exists(p)) {
			if (std::filesystem::is_directory(p))					{ errno = EISDIR;	return (false); }
			if (access(p.c_str(), W_OK))							{ errno = EACCES;	return (false); }
		}

		return (true);
	}

#pragma endregion

#pragma region "Signal"

	bool Utils::valid_signal(const std::string& value) {
		std::set<std::string> validSignals = { "1", "HUP", "SIGHUP", "2", "INT", "SIGINT", "3", "QUIT", "SIGQUIT", "9", "KILL", "SIGKILL", "15", "TERM", "SIGTERM", "10", "USR1", "SIGUSR1", "12", "USR2", "SIGUSR2" };

		return (validSignals.count(toUpper(value)) > 0);
	}

#pragma endregion

#pragma region "Exit Code"

	bool Utils::valid_code(const std::string& value) {
		if (value.empty()) return (false);

		std::istringstream	ss(value);
		std::string			code;

		while (std::getline(ss, code, ',')) {
			code = trim(code);
			if (!valid_number(code, 0, 255)) return (false);
		}

		return (true);
	}

#pragma endregion

#pragma region "Log Level"

	bool Utils::valid_loglevel(const std::string& value) {
		std::set<std::string> validLevels = { "0", "DEBUG", "1", "INFO", "2", "WARN", "WARNING", "3", "ERROR", "4", "CRITICAL" };

		return (validLevels.count(toUpper(value)) > 0);
	}

#pragma endregion

#pragma region "Auto Restart"

	bool Utils::valid_autorestart(const std::string& value) {
		std::string lower = toLower(value);

		return (lower == "true" || lower == "false" || lower == "unexpected" || lower == "yes" || lower == "no" || lower == "1" || lower == "0");
	}

#pragma endregion

#pragma region "Umask"

	bool Utils::valid_umask(const std::string& value) {
		if (value.length() < 1 || value.length() > 4) return (false);

		for (size_t i = 0; i < value.length(); ++i) {
			if (value[i] < '0' || value[i] > '7') return (false);
		}

		return (true);
	}

#pragma endregion

#pragma region "User"

	bool Utils::valid_user(const std::string& value) {
		if (value.empty())						return (false);
		if (toLower(value) == "do not switch")	return (true);

		struct passwd *pw = nullptr;
		char *endptr = nullptr; errno = 0;
		long uid = std::strtol(value.c_str(), &endptr, 10);
		if (!*endptr && errno == 0 && uid >= 0 && uid <= INT_MAX)		pw = getpwuid(static_cast<uid_t>(uid));
		else															pw = getpwnam(value.c_str());

		if (!pw || (pw->pw_shell && std::string(pw->pw_shell).find("nologin") != std::string::npos)) return (false);

		return (true);
	}

#pragma endregion

#pragma region "Chown"

	bool Utils::valid_chown(const std::string& value) {
		if (value.empty())								return (false);
		if (toLower(value) == "do not switch")			return (true);

		size_t colon_pos = value.find(':');
		if (colon_pos == std::string::npos)				return valid_user(value);

		std::string username = value.substr(0, colon_pos);
		std::string group = value.substr(colon_pos + 1);
		if (username.empty() || group.empty())			return (false);

		if (!valid_user(username))						return (false);

		struct passwd *pw = nullptr; char *endptr = nullptr; errno = 0;
		long uid = std::strtol(username.c_str(), &endptr, 10);
		if (*endptr == '\0' && errno == 0 && uid >= 0 && uid <= INT_MAX)	pw = getpwuid(static_cast<uid_t>(uid));
		else																pw = getpwnam(username.c_str());
		if (!pw)										return (false);
		if (pw->pw_shell && std::string(pw->pw_shell).find("nologin") != std::string::npos) return (false);

		struct group *gr = nullptr; endptr = nullptr; errno = 0;
		long gid = std::strtol(group.c_str(), &endptr, 10);
		if (*endptr == '\0' && errno == 0 && gid >= 0 && gid <= INT_MAX)	gr = getgrgid(static_cast<gid_t>(gid));
		else																gr = getgrnam(group.c_str());
		if (!gr)										return (false);

		if (pw->pw_gid == gr->gr_gid)					return (true);

		for (char **member = gr->gr_mem; *member != nullptr; ++member) {
			if (std::string(*member) == pw->pw_name)	return (true);
		}

		int ngroups = 0;
		getgrouplist(pw->pw_name, pw->pw_gid, nullptr, &ngroups);
		if (ngroups > 0) {
			gid_t *groups = new gid_t[ngroups];
			if (getgrouplist(pw->pw_name, pw->pw_gid, groups, &ngroups) != -1) {
				for (int i = 0; i < ngroups; i++) {
					if (groups[i] == gr->gr_gid) { delete[] groups; return (true); }
				}
			}
			delete[] groups;
		}

		return (false);
	}

#pragma endregion

#pragma region "Password"

	bool Utils::valid_password(const std::string& value) {
		if (value.empty()) return (true);

		if (toLower(value.substr(0, 5)) == "{sha}") {
			std::string hash = value.substr(5);
			if (hash.length() != 40) return (false);

			for (char c : hash) {
				if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) return (false);
			}
		}

		return (true);
	}

#pragma endregion

#pragma region "Port"

	bool Utils::valid_port(const std::string& value) {
		size_t colon_pos = value.find(':');
		if (colon_pos == std::string::npos)						return (false);

		std::string host = value.substr(0, colon_pos);
		std::string port = value.substr(colon_pos + 1);

		if (port.empty())										return (false);

		char *endptr = nullptr; errno = 0;
		long nport = std::strtol(port.c_str(), &endptr, 10);
		if (*endptr || errno || nport < 1 || nport > 65535)		return (false);

		if (host.empty() || host == "*")						return (true);
		return (std::regex_match(host, std::regex(R"(^[a-zA-Z0-9][a-zA-Z0-9\.-]*$)")));
	}

#pragma endregion

#pragma region "Server URL"

	bool Utils::valid_serverurl(const std::string &value) {
		if (value.empty())				return (false);
		if (toLower(value) == "auto")	return (true);
	
		static const std::regex pattern(R"(^(https?://[^\s/:]+(:\d+)?(/[^\s]*)?|unix://.+)$)", std::regex::icase);

		if (value.substr(0, 7) == "unix://") return (valid_path(value.substr(7)));

		std::smatch match;
		if (!std::regex_match(value, match, pattern)) return (false);
		
		if (match[2].matched) {
			const std::string	port_str = match[2].str().substr(1);
			char				*endptr = nullptr;

			long port = std::strtol(port_str.c_str(), &endptr, 10);

			if (*endptr || port < 0 || port > 65535) return (false);
		}

		return (true);
	}

#pragma endregion

