/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Validation.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 11:32:25 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/02 00:35:07 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Config.hpp"
	#include "Logging/TaskmasterLog.hpp"

	#include <cstring>															// strerror()
	#include <unistd.h>															// access()
	#include <iostream>															// std::cerr
	#include <filesystem>														// std::filesystem::path(), std::filesystem::is_directory(), std::filesystem::exists()
	#include <pwd.h>															// struct passwd, getpwnam()
	#include <grp.h>															// struct group, getgrnam()
	#include <regex>															// std::regex_match()
	#include <climits>															// INT_MAX

#pragma endregion

#pragma region "Helpers"

	#pragma region "Bool"

		bool ConfigParser::valid_bool(const std::string& value) const {
			std::string lower = toLower(value);
			return (lower == "true" || lower == "false" || lower == "1" || lower == "0" || lower == "yes" || lower == "no");
		}

	#pragma endregion

	#pragma region "Number"

		bool ConfigParser::valid_number(const std::string& value, long min, long max) const {
			if (value.empty()) return (false);

			char	*end;
			long	num = std::strtol(value.c_str(), &end, 10);

			return (*end == '\0' && num >= min && num <= max);
		}

	#pragma endregion

	#pragma region "Path"

		bool ConfigParser::valid_path(const std::string& value, const std::string current_path, bool is_directory, bool allow_auto, bool allow_none) const {
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

		bool ConfigParser::valid_signal(const std::string& value) const {
			std::set<std::string> validSignals = { "1", "HUP", "SIGHUP", "2", "INT", "SIGINT", "3", "QUIT", "SIGQUIT", "9", "KILL", "SIGKILL", "15", "TERM", "SIGTERM", "10", "USR1", "SIGUSR1", "12", "USR2", "SIGUSR2" };

			return (validSignals.count(toUpper(value)) > 0);
		}

	#pragma endregion

	#pragma region "Exit Code"

		bool ConfigParser::valid_code(const std::string& value) const {
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

		bool ConfigParser::valid_loglevel(const std::string& value) const {
			std::set<std::string> validLevels = { "0", "DEBUG", "1", "INFO", "2", "WARN", "WARNING", "3", "ERROR", "4", "CRITICAL" };

			return (validLevels.count(toUpper(value)) > 0);
		}

	#pragma endregion

	#pragma region "Auto Restart"

		bool ConfigParser::valid_autorestart(const std::string& value) const {
			std::string lower = toLower(value);

			return (lower == "true" || lower == "false" || lower == "unexpected" || lower == "yes" || lower == "no" || lower == "1" || lower == "0");
		}

	#pragma endregion

	#pragma region "Umask"

		bool ConfigParser::valid_umask(const std::string& value) const {
			if (value.length() < 1 || value.length() > 4) return (false);

			for (size_t i = 0; i < value.length(); ++i) {
				if (value[i] < '0' || value[i] > '7') return (false);
			}

			return (true);
		}

	#pragma endregion

	#pragma region "User"

		bool ConfigParser::valid_user(const std::string& value) const {
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

		bool ConfigParser::valid_chown(const std::string& value) const {
			if (value.empty()) return (false);

			size_t colon_pos = value.find(':');

			if (colon_pos == std::string::npos) {
				if (toLower(value) == "do not switch")					return (false);
				if (!valid_user(value))									return (false);

				return (true);
			}

			std::string username = value.substr(0, colon_pos);
			std::string group    = value.substr(colon_pos + 1);

			if (toLower(value) == "do not switch")						return (false);
			if (username.empty() || !valid_user(username))				return (false);
			if (group.empty())											return (false);

			struct passwd *pw = nullptr;
			char *endptr = nullptr; errno = 0;
			long uid = std::strtol(username.c_str(), &endptr, 10);
			if (!*endptr && errno == 0 && uid >= 0 && uid <= INT_MAX)	pw = getpwuid(static_cast<uid_t>(uid));
			else														pw = getpwnam(username.c_str());
			if (!pw)													return false;

			struct group *gr = nullptr;
			endptr = nullptr; errno = 0;
			long gid = std::strtol(group.c_str(), &endptr, 10);
			if (!*endptr && errno == 0 && gid >= 0 && gid <= INT_MAX)	gr = getgrgid(static_cast<gid_t>(gid));
			else														gr = getgrnam(group.c_str());
			if (!gr)													return (false);

			if (pw->pw_gid == gr->gr_gid)								return (true);
			for (char **member = gr->gr_mem; *member != nullptr; ++member) {
				if (username == *member) return (true);
			}

			return (false);
		}

	#pragma endregion

	#pragma region "Password"

		bool ConfigParser::valid_password(const std::string& value) const {
			if (value.empty()) return (false);

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

		bool ConfigParser::valid_port(const std::string& value) const {
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

		bool ConfigParser::valid_serverurl(const std::string &value) const {
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

#pragma endregion

#pragma region "Validators"

	#pragma region "Taskmasterd"

		void ConfigParser::validate_taskmasterd() {
			ConfigEntry	*entry;
			std::string	dir, sectionName = "taskmasterd";
			currentSection = sectionName;

			entry = get_value_entry(sectionName, "directory");
			try { entry->value = environment_expand(environment, entry->value); }
			catch (const std::exception& e) {
				error_add(entry->filename, "[" + sectionName + "] directory: unclosed quote or unfinished escape sequence", ERROR, entry->line, entry->order);
				error_add(entry->filename, "[" + sectionName + "] directory: reset to default value: " + defaultValues[sectionName]["directory"], WARNING, 0, entry->order + 1);
				entry->value = defaultValues[sectionName]["directory"];
			}

			if (entry) {
				std::string default_dir = ".";
				if (entry->value != "do not change") {
					if (!valid_path(entry->value, "", true)) {
						error_add(entry->filename, "[" + sectionName + "] directory: invalid path - " + std::string(strerror(errno)), ERROR, entry->line, entry->order);
						if (!valid_path(default_dir, "", true))	error_add(entry->filename, "[" + sectionName + "] directory: failed to use default value - " + std::string(strerror(errno)), CRITICAL, 0, entry->order + 1);
						else {
							entry->value = expand_path(default_dir, "", true, false);
							dir = entry->value;
							error_add(entry->filename, "[" + sectionName + "] directory: reset to default value: " + defaultValues[sectionName]["directory"], WARNING, 0, entry->order + 1);
						}
					} else dir = expand_path(entry->value, "", true, false);
				} else {
					if (!valid_path(default_dir, "", true))	error_add(entry->filename, "[" + sectionName + "] directory: failed to use default value - " + std::string(strerror(errno)), CRITICAL, 0, entry->order + 1);
					else {
						entry->value = expand_path(default_dir, "", true, false);
						dir = entry->value;
					}
				}
			}

			auto it = sections.find(sectionName);
			if (it != sections.end()) {
				for (auto &kv : it->second) {
					const std::string &key = kv.first;
					ConfigEntry &entry = kv.second;
					if (key == "directory") continue;

					try {
						if (key == "environment")	entry.value = environment_expand(environment, entry.value, true);
						else						entry.value = environment_expand(environment, entry.value);
					}
					catch (const std::exception& e) {
						error_add(entry.filename, "[" + sectionName + "] " + key + ": unclosed quote or unfinished escape sequence", ERROR, entry.line, entry.order);
						error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
						entry.value = defaultValues[sectionName][key];
					}

					if (entry.value.empty() && key != "environment") {
						error_add(entry.filename, "[" + sectionName + "] " + key + ": empty value", ERROR, entry.line, entry.order);
						error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
						entry.value = defaultValues[sectionName][key];
					}

					if (key == "nodaemon" && !valid_bool(entry.value)) {
						error_add(entry.filename, "[" + sectionName + "] " + key + ": must be true or false", ERROR, entry.line, entry.order);
						error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
						entry.value = defaultValues[sectionName][key];
					}
					if (key == "silent" && !valid_bool(entry.value)) {
						error_add(entry.filename, "[" + sectionName + "] " + key + ": must be true or false", ERROR, entry.line, entry.order);
						error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
						entry.value = defaultValues[sectionName][key];
					}
					if (key == "strip_ansi" && !valid_bool(entry.value)) {
						error_add(entry.filename, "[" + sectionName + "] " + key + ": must be true or false", ERROR, entry.line, entry.order);
						error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
						entry.value = defaultValues[sectionName][key];
					}
					if (key == "nocleanup" && !valid_bool(entry.value)) {
						error_add(entry.filename, "[" + sectionName + "] " + key + ": must be true or false", ERROR, entry.line, entry.order);
						error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
						entry.value = defaultValues[sectionName][key];
					}
					if (key == "logfile_syslog" && !valid_bool(entry.value)) {
						error_add(entry.filename, "[" + sectionName + "] " + key + ": must be true or false", ERROR, entry.line, entry.order);
						error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
						entry.value = defaultValues[sectionName][key];
					}

					if (key == "logfile_maxbytes") {
						long bytes = parse_size(entry.value);
						if (bytes == -1 || !valid_number(std::to_string(bytes), 0, 1024 * 1024 * 1024)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a value between 0 bytes and 1024 MB", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName][key];
						}
					}
					if (key == "logfile_backups" && !valid_number(entry.value, 0, 1000)) {
						error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a value between 0 and 1000", ERROR, entry.line, entry.order);
						error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
						entry.value = defaultValues[sectionName][key];
					}

					if (key == "loglevel" && !valid_loglevel(entry.value)) {
						error_add(entry.filename, "[" + sectionName + "] " + key + ": must be one of: DEBUG, INFO, WARNING, ERROR, CRITICAL", ERROR, entry.line, entry.order);
						error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
						entry.value = defaultValues[sectionName][key];
					}

					if (key == "logfile") {
						if (toUpper(entry.value) != "NONE") {
							if (!valid_path(entry.value, dir, false)) {
								error_add(entry.filename, "[" + sectionName + "] " + key + ": invalid path - " + std::string(strerror(errno)), ERROR, entry.line, entry.order);
								if (entry.value != defaultValues[sectionName][key]) {
									error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
									entry.value = defaultValues[sectionName][key];
									if (!valid_path(entry.value, dir)) {
										error_add(entry.filename, "[" + sectionName + "] " + key + ": failed to use default value - " + std::string(strerror(errno)), ERROR, entry.line, entry.order);
										entry.value = "NONE";
									}
								} else entry.value = "NONE";
							}
						}
						if (entry.value != "NONE") entry.value = expand_path(entry.value, dir);
					}

					if (key == "pidfile") {
						if (!valid_path(entry.value, dir)) {
							if (entry.value != defaultValues[sectionName][key]) {
								error_add(entry.filename, "[" + sectionName + "] " + key + ": invalid path - " + std::string(strerror(errno)), ERROR, entry.line, entry.order);
								error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
								entry.value = defaultValues[sectionName][key];
								if (!valid_path(entry.value, dir)) {
									error_add(entry.filename, "[" + sectionName + "] " + key + ": failed to use default value - " + std::string(strerror(errno)), CRITICAL, entry.line, entry.order);
								}
							} else error_add(entry.filename, "[" + sectionName + "] " + key + ": invalid path - " + std::string(strerror(errno)), CRITICAL, entry.line, entry.order);
						}
						entry.value = expand_path(entry.value, dir);
					}

					if (key == "childlogdir") {
						if (!valid_path(entry.value, dir, true)) {
							if (entry.value != defaultValues[sectionName][key]) {
								error_add(entry.filename, "[" + sectionName + "] " + key + ": invalid path - " + std::string(strerror(errno)), ERROR, entry.line, entry.order);
								error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
								entry.value = defaultValues[sectionName][key];
								if (!valid_path(entry.value, dir)) {
									error_add(entry.filename, "[" + sectionName + "] " + key + ": failed to use default value - " + std::string(strerror(errno)), ERROR, entry.line, entry.order);
									entry.value = "NONE";
								}
							} else {
								error_add(entry.filename, "[" + sectionName + "] " + key + ": invalid path - " + std::string(strerror(errno)), ERROR, entry.line, entry.order);
								entry.value = "NONE";
							}
						}
						if (entry.value != "NONE") entry.value = expand_path(entry.value, dir, true, false);
					}

					if (key == "user" && !valid_user(entry.value)) {
						error_add(entry.filename, "[" + sectionName + "] " + key + ": invalid user", CRITICAL, entry.line, entry.order);
						entry.value = "";
					}

					if (key == "umask" && !valid_umask(entry.value)) {
						error_add(entry.filename, "[" + sectionName + "] " + key + ": must be in octal format", ERROR, entry.line, entry.order);
						error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
						entry.value = defaultValues[sectionName][key];
					}

					if (key == "minfds") {
						if (!valid_number(entry.value, 1, 65535)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a value between 1 and 65535", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName][key];
						}
						if (check_fd_limit(static_cast<uint16_t>(std::stoul(entry.value)))) {
							if (std::stoul(entry.value) > std::stoul(defaultValues[sectionName][key])) {
								error_add(entry.filename, "[" + sectionName + "] " + key + ": limit could not be applied - system limit too low or insufficient permissions", ERROR, entry.line, entry.order);
								error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
								entry.value = defaultValues[sectionName][key];
								if (check_fd_limit(static_cast<uint16_t>(std::stoul(entry.value)))) {
									error_add(entry.filename, "[" + sectionName + "] " + key + ": limit could not be applied - system limit too low or insufficient permissions", CRITICAL, entry.line, entry.order);
								}
							} else error_add(entry.filename, "[" + sectionName + "] " + key + ": limit could not be applied - system limit too low or insufficient permissions", CRITICAL, entry.line, entry.order);
						}
					}

					if (key == "minprocs") {
						if (!valid_number(entry.value, 1, 10000)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a value between 1 and 10000", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName][key];
						}
						if (check_process_limit(static_cast<uint16_t>(std::stoul(entry.value)))) {
							if (std::stoul(entry.value) > std::stoul(defaultValues[sectionName][key])) {
								error_add(entry.filename, "[" + sectionName + "] " + key + ": limit could not be applied - system limit too low or insufficient permissions", ERROR, entry.line, entry.order);
								error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
								entry.value = defaultValues[sectionName][key];
								if (check_process_limit(static_cast<uint16_t>(std::stoul(entry.value)))) {
									error_add(entry.filename, "[" + sectionName + "] " + key + ": limit could not be applied - system limit too low or insufficient permissions", CRITICAL, entry.line, entry.order);
								}
							} else error_add(entry.filename, "[" + sectionName + "] " + key + ": limit could not be applied - system limit too low or insufficient permissions", CRITICAL, entry.line, entry.order);
						}
					}

					if (key == "logfile_backups" && !valid_number(entry.value, 0, 1000)) {
						error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a value between 0 and 1000", ERROR, entry.line, entry.order);
						error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
						entry.value = defaultValues[sectionName][key];
					}

					if (key == "logfile_backups" && !valid_number(entry.value, 0, 1000)) {
						error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a value between 0 and 1000", ERROR, entry.line, entry.order);
						error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
						entry.value = defaultValues[sectionName][key];
					}

					if (key == "environment" && !environment_validate(entry.value)) {
						error_add(entry.filename, "[" + sectionName + "] " + key + ": invalid variable format", ERROR, entry.line, entry.order);
						entry.value = "";
					}

				}
			}
		}

	#pragma endregion

	#pragma region "Program"

		void ConfigParser::validate_program() {
			std::string	dir;

			dir = get_value("taskmasterd", "directory");

			for (auto& [program, keys] : sections) {
    			if (program.substr(0, 8) == "program:") {
					std::string	sectionName = program;
					currentSection = sectionName;
					ConfigEntry *entry;

					entry = get_value_entry(sectionName, "directory");
					if (entry) {
						try { entry->value = environment_expand(environment, entry->value); }
						catch (const std::exception& e) {
							error_add(entry->filename, "[" + sectionName + "] directory: unclosed quote or unfinished escape sequence", ERROR, entry->line, entry->order);
							error_add(entry->filename, "[" + sectionName + "] directory: reset to default value: " + dir, WARNING, 0, entry->order + 1);
							entry->value = dir;
						}

						if (entry) {
							std::string default_dir = dir;
							if (entry->value != "do not change") {
								if (!valid_path(entry->value, dir, true)) {
									error_add(entry->filename, "[" + sectionName + "] directory: invalid path - " + std::string(strerror(errno)), ERROR, entry->line, entry->order);
									if (!valid_path(default_dir, "", true))	error_add(entry->filename, "[" + sectionName + "] directory: failed to use default value - " + std::string(strerror(errno)), CRITICAL, 0, entry->order + 1);
									else {
										entry->value = expand_path(default_dir, dir, true, false);
										dir = entry->value;
										error_add(entry->filename, "[" + sectionName + "] directory: reset to default value: " + defaultValues[sectionName]["directory"], WARNING, 0, entry->order + 1);
									}
								} else dir = expand_path(entry->value, dir, true, false);
							} else {
								if (!valid_path(default_dir, dir, true))	error_add(entry->filename, "[" + sectionName + "] directory: failed to use default value - " + std::string(strerror(errno)), CRITICAL, 0, entry->order + 1);
								else {
									entry->value = expand_path(default_dir, dir, true, false);
									dir = entry->value;
								}
							}
						}
					}

					// Error de segfault y cambiar mensajes de error
					// Try catch en environment_expand() que faltan
					entry = get_value_entry(sectionName, "command");
					if (!entry) {
						error_add(entry->filename, "[" + sectionName + "] command: required", ERROR, entry->line, entry->order);
					} else {
						try { entry->value = environment_expand(environment, entry->value); }
						catch (const std::exception& e) { error_add(entry->filename, "[" + sectionName + "] directory: unclosed quote or unfinished escape sequence", ERROR, entry->line, entry->order); }

						if (entry->value.empty()) {
							error_add(entry->filename, "[" + sectionName + "] command: empty value", ERROR, entry->line, entry->order);
						} else if (!command_executable(entry->value, entry->value)) {
							error_add(entry->filename, "[" + sectionName + "] command: must be a valid executable", ERROR, entry->line, entry->order);
						}
					}

					for (auto &kv : keys) {
						const std::string &key = kv.first;
						ConfigEntry &entry = kv.second;

						if (key == "command") continue;

						try {
							if (key == "environment")	entry.value = environment_expand(environment, entry.value, true);
							else						entry.value = environment_expand(environment, entry.value);
						}
						catch (const std::exception& e) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": unclosed quote or unfinished escape sequence", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName][key];
						}

						if (entry.value.empty() && key != "environment") {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": empty value", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName][key];
						}

						if (key == "tty_mode" && !valid_bool(entry.value)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": must be true or false", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName][key];
						}
						if (key == "autostart" && !valid_bool(entry.value)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": must be true or false", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName][key];
						}
						if (key == "stopasgroup" && !valid_bool(entry.value)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": must be true or false", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName][key];
						}
						if (key == "killasgroup" && !valid_bool(entry.value)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": must be true or false", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName][key];
						}
						if (key == "redirect_stderr" && !valid_bool(entry.value)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": must be true or false", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName][key];
						}
						if (key == "stdout_logfile_syslog" && !valid_bool(entry.value)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": must be true or false", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName][key];
						}
						if (key == "stderr_logfile_syslog" && !valid_bool(entry.value)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": must be true or false", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName][key];
						}

						if (key == "priority" && !valid_number(entry.value, 0, 999)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a value between 0 and 999", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName][key];
						}
						if (key == "startsecs" && !valid_number(entry.value, 0, 3600)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a value between 0 and 3600", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName][key];
						}
						if (key == "startretries" && !valid_number(entry.value, 0, 100)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a value between 0 and 100", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName][key];
						}
						if (key == "stopwaitsecs" && !valid_number(entry.value, 1, 3600)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": mmust be a value between 1 and 3600", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName][key];
						}

						if (key == "stdout_logfile_maxbytes") {
							long bytes = parse_size(entry.value);
							if (bytes == -1 || !valid_number(std::to_string(bytes), 0, 1024 * 1024 * 1024)) {
								error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a value between 0 bytes and 1024 MB", ERROR, entry.line, entry.order);
								error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
								entry.value = defaultValues[sectionName][key];
							}
						}
						if (key == "stdout_logfile_backups" && !valid_number(entry.value, 0, 1000)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a value between 0 and 1000", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName][key];
						}

						if (key == "stderr_logfile_maxbytes") {
							long bytes = parse_size(entry.value);
							if (bytes == -1 || !valid_number(std::to_string(bytes), 0, 1024 * 1024 * 1024)) {
								error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a value between 0 bytes and 1024 MB", ERROR, entry.line, entry.order);
								error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
								entry.value = defaultValues[sectionName][key];
							}
						}
						if (key == "stderr_logfile_backups" && !valid_number(entry.value, 0, 1000)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a value between 0 and 1000", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName][key];
						}

						if (key == "process_name") {
							std::string numprocs =  environment_expand(environment, get_value(sectionName, "numprocs"));
							if (numprocs != "1" && entry.value.find("${PROCESS_NUM:") == std::string::npos && entry.value.find("${PROCESS_NUM}") == std::string::npos && entry.value.find("$PROCESS_NUM") == std::string::npos) {
								error_add(entry.filename, "[" + sectionName + "] " + key + ": must include $PROCESS_NUM when 'numprocs' is greater than 1", ERROR, entry.line, entry.order);
							}
						}

						if (key == "numprocs") {
							if (!valid_number(entry.value, 1, 1000))
								error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a value between 1 and 1000", ERROR, entry.line, entry.order);
							if (entry.value != "1") {
								std::string process_name = environment_expand(environment, get_value(sectionName, "process_name"));
								if (process_name.empty() || (process_name.find("${PROCESS_NUM:") == std::string::npos && process_name.find("${PROCESS_NUM}") == std::string::npos && process_name.find("$PROCESS_NUM") == std::string::npos))
									error_add(entry.filename, "[" + sectionName + "] " + key + ": 'process_name' must include $PROCESS_NUM when 'numprocs' is greater than 1", ERROR, entry.line, entry.order);
							}
						}

						if (key == "autorestart" && !valid_autorestart(entry.value)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": must be false, unexpected or true", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName][key];
						}

						if (key == "exitcodes" && !valid_code(entry.value)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": must be comma-separated numbers between 0 and 255", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName][key];
						}

						if (key == "stopsignal" && !valid_signal(entry.value)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a valid signal (HUP, INT, QUIT, KILL, TERM, USR1, USR2)", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName][key];
						}

						if (key == "user" && !valid_user(entry.value)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": invalid user", CRITICAL, entry.line, entry.order);
							entry.value = "";
						}

						if (key == "umask") {
							if (entry.value == "inherit") entry.value = defaultValues["taskmasterd"][key];
							if (!valid_umask(entry.value)) {
								error_add(entry.filename, "[" + sectionName + "] " + key + ": must be in octal format", ERROR, entry.line, entry.order);
								error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
								entry.value = defaultValues[sectionName][key];
							}
						}

						if (key == "stdout_logfile") {
							if (toUpper(entry.value) == "AUTO") {
								std::string childlogdir = get_value("taskmasterd", "childlogdir");
								entry.value = sectionName.substr(8) + "_stdout.log";
								if (!valid_path(entry.value, childlogdir)) {
									error_add(entry.filename, "[" + sectionName + "] " + key + ": invalid path - " + std::string(strerror(errno)), ERROR, entry.line, entry.order);
									entry.value = "NONE";
								}
							} else if (toUpper(entry.value) != "NONE") {
								if (!valid_path(entry.value, dir)) {
									error_add(entry.filename, "[" + sectionName + "] " + key + ": invalid path - " + std::string(strerror(errno)), ERROR, entry.line, entry.order);
									if (entry.value != defaultValues[sectionName][key]) {
										error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
										entry.value = defaultValues[sectionName][key];
										if (!valid_path(entry.value, dir)) {
											error_add(entry.filename, "[" + sectionName + "] " + key + ": failed to use default value - " + std::string(strerror(errno)), ERROR, entry.line, entry.order);
											entry.value = "NONE";
										}
									} else entry.value = "NONE";
								}
							}
							if (entry.value != "NONE") entry.value = expand_path(entry.value, dir);
						}

						if (key == "stderr_logfile") {
							if (toUpper(entry.value) == "AUTO") {
								std::string childlogdir = get_value("taskmasterd", "childlogdir");
								entry.value = sectionName.substr(8) + "_stdout.log";
								if (!valid_path(entry.value, childlogdir)) {
									error_add(entry.filename, "[" + sectionName + "] " + key + ": invalid path - " + std::string(strerror(errno)), ERROR, entry.line, entry.order);
									entry.value = "NONE";
								}
							} else if (toUpper(entry.value) != "NONE") {
								if (!valid_path(entry.value, dir)) {
									error_add(entry.filename, "[" + sectionName + "] " + key + ": invalid path - " + std::string(strerror(errno)), ERROR, entry.line, entry.order);
									if (entry.value != defaultValues[sectionName][key]) {
										error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
										entry.value = defaultValues[sectionName][key];
										if (!valid_path(entry.value, dir)) {
											error_add(entry.filename, "[" + sectionName + "] " + key + ": failed to use default value - " + std::string(strerror(errno)), ERROR, entry.line, entry.order);
											entry.value = "NONE";
										}
									} else entry.value = "NONE";
								}
							}
							if (entry.value != "NONE") entry.value = expand_path(entry.value, dir);
						}

						// AUTO - Arreglar
						if (key == "serverurl" && !valid_serverurl(entry.value)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": invalid format", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName][key];
						}

						if (key == "environment" && !environment_validate(entry.value)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": invalid variable format", ERROR, entry.line, entry.order);
							entry.value = "";
						}
					}
				}
			}
		}

	#pragma endregion

	#pragma region "Group"

		void ConfigParser::validate_group() {
			std::set<std::string> programs;
			for (auto& [program, keys] : sections) {
				if (program.substr(0, 8) == "program:") {
					if (!get_value(program, "command").empty()) programs.insert(program.substr(8));
				}
			}

			for (auto& [group, keys] : sections) {
    			if (group.substr(0, 6) == "group:") {
					std::string	sectionName = group;
					currentSection = sectionName;

					for (auto &kv : keys) {
						const std::string &key = kv.first;
						ConfigEntry &entry = kv.second;

						try { entry.value = environment_expand(environment, entry.value); }
						catch (const std::exception& e) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": unclosed quote or unfinished escape sequence", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName][key];
						}

						if (entry.value.empty()) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": empty value", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName][key];
						}

						if (key == "programs") {
							std::string name;
							std::istringstream cm_stream(entry.value);
							while (std::getline(cm_stream, name, ',')) {
								if (!programs.count(trim(name))) error_add(entry.filename, "[" + sectionName + "] " + key + ": " + trim(name) + " does not exist or is not configured correctly", ERROR, entry.line, entry.order);
							}
						}

						if (key == "priority" && !valid_number(entry.value, 0, 999)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a value between 0 and 999", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName][key];
						}
					}
				}
			}
		}

	#pragma endregion

	#pragma region "Unix Server"

		void ConfigParser::validate_unix_server() {
			std::string	dir, sectionName = "unix_http_server";
			currentSection = sectionName;

			dir = get_value(sectionName, "directory");

			auto it = sections.find(sectionName);
			if (it != sections.end()) {
				for (auto &kv : it->second) {
					const std::string &key = kv.first;
					ConfigEntry &entry = kv.second;

					try { entry.value = environment_expand(environment, entry.value); }
					catch (const std::exception& e) {
						error_add(entry.filename, "[" + sectionName + "] " + key + ": unclosed quote or unfinished escape sequence", ERROR, entry.line, entry.order);
						error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
						entry.value = defaultValues[sectionName][key];
					}

					if (entry.value.empty() && key != "username" && key != "password") {
						error_add(entry.filename, "[" + sectionName + "] " + key + ": empty value", ERROR, entry.line, entry.order);
						error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
						entry.value = defaultValues[sectionName][key];
					}

					if (key == "file") {
						if (!valid_path(entry.value, dir)) {
							if (entry.value != defaultValues[sectionName][key]) {
								error_add(entry.filename, "[" + sectionName + "] " + key + ": invalid path - " + std::string(strerror(errno)), ERROR, entry.line, entry.order);
								error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
								entry.value = defaultValues[sectionName][key];
								if (!valid_path(entry.value, dir)) {
									error_add(entry.filename, "[" + sectionName + "] " + key + ": failed to use default value - " + std::string(strerror(errno)), ERROR, entry.line, entry.order);
									entry.value = "NONE";
								}
							} else {
								error_add(entry.filename, "[" + sectionName + "] " + key + ": invalid path - " + std::string(strerror(errno)), ERROR, entry.line, entry.order);
								entry.value = "NONE";
							}
						}
						if (toUpper(entry.value) != "NONE") entry.value = expand_path(entry.value, dir);
					}

					if (key == "chmod" && !valid_umask(entry.value)) {
						error_add(entry.filename, "[" + sectionName + "] " + key + ": must be in octal format", ERROR, entry.line, entry.order);
						error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
						entry.value = defaultValues[sectionName][key];
					}

					if (key == "chown" && !valid_chown(entry.value)) {
						error_add(entry.filename, "[" + sectionName + "] " + key + ": invalid user or group", ERROR, entry.line, entry.order);
						entry.value = "";
					}

					if (key == "password" && !valid_password(entry.value)) {
						error_add(entry.filename, "[" + sectionName + "] " + key + ": invalid SHA format", ERROR, entry.line, entry.order);
						entry.value = "";
					}
				}
			}
		}

	#pragma endregion

	#pragma region "Inet Server"

		void ConfigParser::validate_inet_server() {
			std::string	sectionName = "inet_http_server";
			currentSection = sectionName;

			auto it = sections.find(sectionName);
			if (it != sections.end()) {
				for (auto &kv : it->second) {
					const std::string &key = kv.first;
					ConfigEntry &entry = kv.second;

					try { entry.value = environment_expand(environment, entry.value); }
					catch (const std::exception& e) {
						error_add(entry.filename, "[" + sectionName + "] " + key + ": unclosed quote or unfinished escape sequence", ERROR, entry.line, entry.order);
						error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
						entry.value = defaultValues[sectionName][key];
					}

					if (entry.value.empty() && key != "username" && key != "password") {
						error_add(entry.filename, "[" + sectionName + "] " + key + ": empty value", ERROR, entry.line, entry.order);
						error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
						entry.value = defaultValues[sectionName][key];
					}

					if (key == "port" && !valid_port(entry.value)) {
						error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a valid TCP host:port", ERROR, entry.line, entry.order);
						entry.value = "";
					}

					if (key == "password" && !valid_password(entry.value)) {
						error_add(entry.filename, "[" + sectionName + "] " + key + ": invalid SHA format", ERROR, entry.line, entry.order);
						entry.value = "";
					}
				}
			}
		}

	#pragma endregion

	#pragma region "Options"

		int ConfigParser::validate_options(ConfigOptions& Options) const {
			static std::string	dir;
			std::string			errors;

			if (Options.options.find_first_of('d') != std::string::npos) {
				if (!valid_path(Options.directory, dir, true))
					errors += "directory: path is invalid\n";
				dir = expand_path(Options.directory, "", true, false);
			}

			if (Options.options.find_first_of('c') != std::string::npos) {
				if (!Options.configuration.empty() && expand_path(Options.configuration, "", true, false).empty())
					errors += "configuration: cannot open config file\n";
			}

			// if (Options.options.find_first_of('u') != std::string::npos) {
			// 	try { validate_taskmasterd("", "user", Options.user); }
			// 	catch (const std::exception& e) { errors += std::string(e.what()).substr(3) + "\n"; }
			// }

			// if (Options.options.find_first_of('m') != std::string::npos) {
			// 	try { validate_taskmasterd("", "umask", Options.umask); }
			// 	catch (const std::exception& e) { errors += std::string(e.what()).substr(3) + "\n"; }
			// }

			// if (Options.options.find_first_of('l') != std::string::npos) {
			// 	if (!valid_path(Options.logfile, dir, false, false, true))
			// 		errors += "logfile: path is invalid - " + std::string(strerror(errno)) + "\n";
			// }

			// if (Options.options.find_first_of('y') != std::string::npos) {
			// 	try { validate_taskmasterd("", "logfile_maxbytes", Options.logfile_maxbytes); }
			// 	catch (const std::exception& e) { errors += std::string(e.what()).substr(3) + "\n"; }
			// }

			// if (Options.options.find_first_of('z') != std::string::npos) {
			// 	try { validate_taskmasterd("", "logfile_backups", Options.logfile_backups); }
			// 	catch (const std::exception& e) { errors += std::string(e.what()).substr(3) + "\n"; }
			// }

			// if (Options.options.find_first_of('e') != std::string::npos) {
			// 	try { validate_taskmasterd("", "loglevel", Options.loglevel); }
			// 	catch (const std::exception& e) { errors += std::string(e.what()).substr(3) + "\n"; }
			// }

			// if (Options.options.find_first_of('j') != std::string::npos) {
			// 	if (!valid_path(Options.pidfile, dir))
			// 		errors += "pidfile: path is invalid\n";
			// }

			// if (Options.options.find_first_of('q') != std::string::npos) {
			// 	if (!valid_path(Options.childlogdir, dir, true))
			// 		errors += "childlogdir: path is invalid\n";
			// }

			// if (Options.options.find_first_of('a') != std::string::npos) {
			// 	try { validate_taskmasterd("", "minfds", Options.minfds); }
			// 	catch (const std::exception& e) { errors += std::string(e.what()).substr(3) + "\n"; }
			// }

			// if (Options.options.find_first_of('p') != std::string::npos) {
			// 	try { validate_taskmasterd("", "minprocs", Options.minprocs); }
			// 	catch (const std::exception& e) { errors += std::string(e.what()).substr(3) + "\n"; }
			// }

			if (!errors.empty()) { std::cerr << Options.fullName << ": invalid options: \n\n" <<  errors; return (2); }

			return (0);
		}

	#pragma endregion

#pragma endregion

#pragma region "Validate"

	void ConfigParser::validate() {
		validate_taskmasterd();
		validate_program();
		validate_group();
		validate_unix_server();
		validate_inet_server();
	}

#pragma endregion

#pragma region "Errors"

	#pragma region "Add"

		void ConfigParser::error_add(std::string& filename, std::string msg, uint8_t level, uint16_t line, uint16_t order) {
			errors.push_back({filename, msg, level, line, order});
		}

	#pragma endregion

	#pragma region "Print"

		void ConfigParser::error_print() {
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

				if (maxLevel > error_maxLevel) error_maxLevel = maxLevel;
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

#pragma endregion

		// char hostname[255];
		// environment_add(environment_config, "HOST_NAME", (!gethostname(hostname, sizeof(hostname))) ? std::string(hostname) : "unknown");
		// environment_add(environment_config, "HERE", configPath.parent_path());
		// environment_initialize(environment);
// environment_add(environment_config, "HERE", configPath.parent_path());

		// if (currentSection.substr(0, 8) == "program:") {
		// 	std::string program_name = trim(currentSection.substr(8));
		// 	if (!program_name.empty()) environment_add(environment_config, "PROGRAM_NAME", program_name);
		// }
		// else if (currentSection.substr(0, 6) == "group:") {
		// 	std::string group_name = trim(currentSection.substr(7));
		// 	if (!group_name.empty()) environment_add(environment_config, "GROUP_NAME", group_name);
		// } else {
		// 	environment_del(environment_config, "PROGRAM_NAME");
		// 	environment_del(environment_config, "GROUP_NAME");
		// }

		
		// std::string expanded_value;
		// std::map<std::string, std::string> temp = environment;
		// environment_add(temp, environment_config, true);
		// expanded_value = environment_expand(temp, value, key == "environment");

		// if (expanded_value.empty())								throw std::runtime_error("[" + currentSection + "] " + key + ": empty value");

		// validate(currentSection, key, expanded_value);
	// #include <unistd.h>															// gethostname()