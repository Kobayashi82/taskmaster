/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Validation.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 11:32:25 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/03 19:08:11 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Config.hpp"
	#include "Logging/TaskmasterLog.hpp"

	#include <cstring>															// strerror()
	#include <unistd.h>															// access(), gethostname()
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
			std::string	dir = ".", sectionName = "taskmasterd";
			currentSection = sectionName;

			std::string HERE = environment_get(environment, "HERE");
			environment_add(environment, "HERE", expand_path(".", "", true, false));

			entry = get_value_entry(sectionName, "directory");
			if (entry) {
				try { entry->value = environment_expand(environment, entry->value); }
				catch (const std::exception& e) {
					error_add(entry->filename, "[" + sectionName + "] directory: unclosed quote or unfinished escape sequence", ERROR, entry->line, entry->order);
					error_add(entry->filename, "[" + sectionName + "] directory: reset to default value: " + defaultValues[sectionName]["directory"], WARNING, 0, entry->order + 1);
					entry->value = defaultValues[sectionName]["directory"];
				}

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
						if (key == "environment")	entry.value = environment_expand(environment, entry.value, ",");
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
						if (toUpper(entry.value) != "NONE") entry.value = expand_path(entry.value, dir);
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

					if (key == "user") {
						if (!valid_user(entry.value)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": invalid user", CRITICAL, entry.line, entry.order);
							entry.value = "";
						} else {
							if (!is_root && toLower(entry.value) != "do not switch") {
								struct passwd* pw = getpwuid(getuid());
								if (pw && pw->pw_name != entry.value && entry.value != std::to_string(getuid()))
									error_add(entry.filename, "[" + sectionName + "] " + key + ": cannot drop privileges, not running as root", CRITICAL, entry.line, entry.order);
							}
						}
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

			if (!HERE.empty()) environment_add(environment, "HERE", HERE);

			std::string user = get_value("taskmasterd", "user");
			if (is_root && (user.empty() || toLower(user) == "do not switch")) {
				Log.warning("taskmasterd is running as root. Privileges were not dropped because no user is specified in the config file. If you intend to run as root, you can set user=root in the config file to avoid this message.");
				error_maxLevel = WARNING;
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

					uint16_t	last_order = 0;
					std::string	filename;

					auto itSec = sections.find(program);
					if (itSec != sections.end() && !itSec->second.empty()) {
						filename = itSec->second.begin()->second.filename;
						order = itSec->second.begin()->second.order;
					}

					std::string HERE = environment_get(environment, "HERE");
					environment_add(environment, "HERE", std::filesystem::path(filename).parent_path());

					std::string numprocs = "1";
					entry = get_value_entry(sectionName, "numprocs");
					if (entry) {
						try { entry->value = environment_expand(environment, entry->value); }
						catch (const std::exception& e) {
							error_add(entry->filename, "[" + sectionName + "] numprocs: unclosed quote or unfinished escape sequence", ERROR, entry->line, entry->order);
							error_add(entry->filename, "[" + sectionName + "] numprocs: reset to default value: " + defaultValues[sectionName.substr(0, 8)]["numprocs"], WARNING, 0, entry->order + 1);
							entry->value = defaultValues[sectionName.substr(0, 8)]["numprocs"];
						}
						if (!valid_number(entry->value, 0, 100)) {
							error_add(entry->filename, "[" + sectionName + "] " + "numprocs: must be a value between 0 and 100", ERROR, entry->line, entry->order);
							error_add(entry->filename, "[" + sectionName + "] " + "numprocs: reset to default value: " + defaultValues[sectionName.substr(0, 8)]["numprocs"], WARNING, 0, entry->order + 1);
							entry->value = defaultValues[sectionName.substr(0, 8)]["numprocs"];
						}
					}

					std::string PROGRAM_NAME	= environment_get(environment, "PROGRAM_NAME");
					std::string NUMPROCS		= environment_get(environment, "NUMPROCS");

					environment_add(environment, "PROGRAM_NAME", program.substr(8));
					environment_add(environment, "NUMPROCS", numprocs);

					entry = get_value_entry(sectionName, "directory");
					if (entry) {
						try { entry->value = environment_expand(environment, entry->value); }
						catch (const std::exception& e) {
							error_add(entry->filename, "[" + sectionName + "] directory: unclosed quote or unfinished escape sequence", ERROR, entry->line, entry->order);
							error_add(entry->filename, "[" + sectionName + "] directory: reset to default value: " + dir, WARNING, 0, entry->order + 1);
							entry->value = dir;
						}

						std::string default_dir = dir;
						if (entry->value != "do not change") {
							if (!valid_path(entry->value, dir, true)) {
								error_add(entry->filename, "[" + sectionName + "] directory: invalid path - " + std::string(strerror(errno)), ERROR, entry->line, entry->order);
								if (!valid_path(default_dir, "", true))	error_add(entry->filename, "[" + sectionName + "] directory: failed to use default value - " + std::string(strerror(errno)), CRITICAL, 0, entry->order + 1);
								else {
									entry->value = expand_path(default_dir, dir, true, false);
									dir = entry->value;
									error_add(entry->filename, "[" + sectionName + "] directory: reset to default value: " + defaultValues[sectionName.substr(0, 8)]["directory"], WARNING, 0, entry->order + 1);
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

					for (auto &kv : keys) {
						const std::string &key = kv.first;
						ConfigEntry &entry = kv.second;

						if (entry.order > last_order) last_order = entry.order;
						filename = entry.filename;

						if (key == "command") continue;

						try {
							if (key == "environment")	entry.value = environment_expand(environment, entry.value, ",");
							else						entry.value = environment_expand(environment, entry.value);
						}
						catch (const std::exception& e) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": unclosed quote or unfinished escape sequence", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName.substr(0, 8)][key];
						}

						if (entry.value.empty() && key != "environment") {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": empty value", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName.substr(0, 8)][key];
						}

						if (key == "tty_mode" && !valid_bool(entry.value)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": must be true or false", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName.substr(0, 8)][key];
						}
						if (key == "autostart" && !valid_bool(entry.value)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": must be true or false", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName.substr(0, 8)][key];
						}
						if (key == "stopasgroup" && !valid_bool(entry.value)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": must be true or false", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName.substr(0, 8)][key];
						}
						if (key == "killasgroup" && !valid_bool(entry.value)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": must be true or false", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName.substr(0, 8)][key];
						}
						if (key == "redirect_stderr" && !valid_bool(entry.value)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": must be true or false", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName.substr(0, 8)][key];
						}
						if (key == "stdout_logfile_syslog" && !valid_bool(entry.value)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": must be true or false", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName.substr(0, 8)][key];
						}
						if (key == "stderr_logfile_syslog" && !valid_bool(entry.value)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": must be true or false", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName.substr(0, 8)][key];
						}

						if (key == "priority" && !valid_number(entry.value, 0, 999)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a value between 0 and 999", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName.substr(0, 8)][key];
						}
						if (key == "startsecs" && !valid_number(entry.value, 0, 3600)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a value between 0 and 3600", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName.substr(0, 8)][key];
						}
						if (key == "startretries" && !valid_number(entry.value, 0, 100)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a value between 0 and 100", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName.substr(0, 8)][key];
						}
						if (key == "stopwaitsecs" && !valid_number(entry.value, 1, 3600)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": mmust be a value between 1 and 3600", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName.substr(0, 8)][key];
						}

						if (key == "stdout_logfile_maxbytes") {
							long bytes = parse_size(entry.value);
							if (bytes == -1 || !valid_number(std::to_string(bytes), 0, 1024 * 1024 * 1024)) {
								error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a value between 0 bytes and 1024 MB", ERROR, entry.line, entry.order);
								error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
								entry.value = defaultValues[sectionName.substr(0, 8)][key];
							}
						}
						if (key == "stdout_logfile_backups" && !valid_number(entry.value, 0, 1000)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a value between 0 and 1000", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName.substr(0, 8)][key];
						}

						if (key == "stderr_logfile_maxbytes") {
							long bytes = parse_size(entry.value);
							if (bytes == -1 || !valid_number(std::to_string(bytes), 0, 1024 * 1024 * 1024)) {
								error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a value between 0 bytes and 1024 MB", ERROR, entry.line, entry.order);
								error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
								entry.value = defaultValues[sectionName.substr(0, 8)][key];
							}
						}
						if (key == "stderr_logfile_backups" && !valid_number(entry.value, 0, 1000)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a value between 0 and 1000", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName.substr(0, 8)][key];
						}

						if (key == "process_name") {
							std::string numprocs;
							try { numprocs =  environment_expand(environment, get_value(sectionName, "numprocs")); }
							catch (const std::exception& e) { numprocs = "0"; }
							if (numprocs != "1" && entry.value.find("${PROCESS_NUM:") == std::string::npos && entry.value.find("${PROCESS_NUM}") == std::string::npos && entry.value.find("$PROCESS_NUM") == std::string::npos
								&& entry.value.find("${EX_PROCESS_NUM:") == std::string::npos && entry.value.find("${EX_PROCESS_NUM}") == std::string::npos && entry.value.find("$EX_PROCESS_NUM") == std::string::npos) {
								error_add(entry.filename, "[" + sectionName + "] " + key + ": must include $PROCESS_NUM when 'numprocs' is greater than 1", ERROR, entry.line, entry.order);
							}
						}

						if (key == "numprocs") {
							if (!valid_number(entry.value, 1, 1000))
								error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a value between 1 and 1000", ERROR, entry.line, entry.order);
							if (entry.value != "1") {
								std::string process_name;
								try { process_name = environment_expand(environment, get_value(sectionName, "process_name")); }
								catch (const std::exception& e) { process_name = ""; }
								if (process_name.empty() || (process_name.find("${PROCESS_NUM:") == std::string::npos && process_name.find("${PROCESS_NUM}") == std::string::npos && process_name.find("$PROCESS_NUM") == std::string::npos
									&& process_name.find("${EX_PROCESS_NUM:") == std::string::npos && process_name.find("${EX_PROCESS_NUM}") == std::string::npos && process_name.find("$EX_PROCESS_NUM") == std::string::npos))
									error_add(entry.filename, "[" + sectionName + "] " + key + ": 'process_name' must include $PROCESS_NUM when 'numprocs' is greater than 1", ERROR, entry.line, entry.order);
							}
						}

						if (key == "numprocs_start" && !valid_number(entry.value, 0, 65535)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a value between 0 and 65535", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName.substr(0, 8)][key];
						}

						if (key == "autorestart" && !valid_autorestart(entry.value)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": must be false, unexpected or true", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName.substr(0, 8)][key];
						}

						if (key == "exitcodes" && !valid_code(entry.value)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": must be comma-separated numbers between 0 and 255", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName.substr(0, 8)][key];
						}

						if (key == "stopsignal" && !valid_signal(entry.value)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a valid signal (HUP, INT, QUIT, KILL, TERM, USR1, USR2)", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName.substr(0, 8)][key];
						}

						if (key == "user") {
							if (!valid_user(entry.value)) {
								error_add(entry.filename, "[" + sectionName + "] " + key + ": invalid user", CRITICAL, entry.line, entry.order);
								entry.value = "";
							} else {
								if (!is_root && toLower(entry.value) != "do not switch") {
									struct passwd* pw = getpwuid(getuid());
									if (pw && pw->pw_name != entry.value && entry.value != std::to_string(getuid()))
										error_add(entry.filename, "[" + sectionName + "] " + key + ": cannot switch user, not running as root", CRITICAL, entry.line, entry.order);
								}
							}
						}

						if (key == "umask") {
							if (entry.value == "inherit") entry.value = defaultValues["taskmasterd"][key];
							if (!valid_umask(entry.value)) {
								error_add(entry.filename, "[" + sectionName + "] " + key + ": must be in octal format", ERROR, entry.line, entry.order);
								error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
								entry.value = defaultValues[sectionName.substr(0, 8)][key];
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
									if (entry.value != defaultValues[sectionName.substr(0, 8)][key]) {
										error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
										entry.value = defaultValues[sectionName.substr(0, 8)][key];
										if (!valid_path(entry.value, dir)) {
											error_add(entry.filename, "[" + sectionName + "] " + key + ": failed to use default value - " + std::string(strerror(errno)), ERROR, entry.line, entry.order);
											entry.value = "NONE";
										}
									} else entry.value = "NONE";
								}
							}
							if (toUpper(entry.value) != "NONE") entry.value = expand_path(entry.value, dir);
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
									if (entry.value != defaultValues[sectionName.substr(0, 8)][key]) {
										error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
										entry.value = defaultValues[sectionName.substr(0, 8)][key];
										if (!valid_path(entry.value, dir)) {
											error_add(entry.filename, "[" + sectionName + "] " + key + ": failed to use default value - " + std::string(strerror(errno)), ERROR, entry.line, entry.order);
											entry.value = "NONE";
										}
									} else entry.value = "NONE";
								}
							}
							if (toUpper(entry.value) != "NONE") entry.value = expand_path(entry.value, dir);
						}

						if (key == "serverurl") {
							if (toUpper(entry.value) != "AUTO" && !valid_serverurl(entry.value)) {
								error_add(entry.filename, "[" + sectionName + "] " + key + ": invalid format", ERROR, entry.line, entry.order);
								error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
								entry.value = defaultValues[sectionName.substr(0, 8)][key];
							}
							if (toUpper(entry.value) == "AUTO") {
								std::string url;
								if (has_section("unix_http_server")) {
									url = get_value("unix_http_server", "file");
									if (!url.empty()) entry.value = "unix://" + url;
								}
								if (url.empty() && has_section("inet_http_server")) {
									url = get_value("inet_http_server", "port");
									if (!url.empty()) entry.value = "http://" + url;
								}
							}
						}

						if (key == "environment" && !environment_validate(entry.value)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": invalid variable format", ERROR, entry.line, entry.order);
							entry.value = "";
						}
					}

					entry = get_value_entry(sectionName, "command");
					if (entry) {
						try { entry->value = environment_expand(environment, entry->value); }
						catch (const std::exception& e) { error_add(entry->filename, "[" + sectionName + "] command: unclosed quote or unfinished escape sequence", ERROR, entry->line, entry->order); }

						if (entry->value.empty()) {
							error_add(entry->filename, "[" + sectionName + "] command: empty value", ERROR, entry->line, entry->order);
						} else if (!command_executable(entry->value, entry->value)) {
							error_add(entry->filename, "[" + sectionName + "] command: must be a valid executable", ERROR, entry->line, entry->order);
						}
					} else error_add(filename, "[" + sectionName + "] command: required", ERROR, 0, last_order);

					if (!HERE.empty())			environment_add(environment, "HERE", HERE);
					if (!PROGRAM_NAME.empty())	environment_add(environment, "PROGRAM_NAME", PROGRAM_NAME);
					if (!NUMPROCS.empty())		environment_add(environment, "NUMPROCS", NUMPROCS);
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

					uint16_t	last_order = 0;
					std::string	filename;

					auto itSec = sections.find(group);
					if (itSec != sections.end() && !itSec->second.empty()) {
						filename = itSec->second.begin()->second.filename;
						order = itSec->second.begin()->second.order;
					}

					std::string HERE		= environment_get(environment, "HERE");
					std::string GROUP_NAME	= environment_get(environment, "GROUP_NAME");

					environment_add(environment, "HERE", std::filesystem::path(filename).parent_path());
					environment_add(environment, "GROUP_NAME", group.substr(6));

					for (auto &kv : keys) {
						const std::string &key = kv.first;
						ConfigEntry &entry = kv.second;

						try { entry.value = environment_expand(environment, entry.value); }
						catch (const std::exception& e) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": unclosed quote or unfinished escape sequence", ERROR, entry.line, entry.order);
							if (key != "programs") {
								error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 6)][key], WARNING, 0, entry.order + 1);
								entry.value = defaultValues[sectionName.substr(0, 6)][key];
							} else entry.value = "";
						}

						if (entry.value.empty()) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": empty value", ERROR, entry.line, entry.order);
							if (key != "programs") {
								error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 6)][key], WARNING, 0, entry.order + 1);
								entry.value = defaultValues[sectionName.substr(0, 6)][key];
							}
						}

						if (key == "programs" && !entry.value.empty()) {
							std::string name;
							std::istringstream cm_stream(entry.value);
							while (std::getline(cm_stream, name, ',')) {
								if (!programs.count(trim(name))) error_add(entry.filename, "[" + sectionName + "] " + key + ": " + trim(name) + " does not exist or is not configured correctly", ERROR, entry.line, entry.order);
							}
						}

						if (key == "priority" && !valid_number(entry.value, 0, 999)) {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a value between 0 and 999", ERROR, entry.line, entry.order);
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 6)][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName.substr(0, 6)][key];
						}
					}

					ConfigEntry	*entry;
					entry = get_value_entry(sectionName, "programs");
					if (!entry) error_add(filename, "[" + sectionName + "] programs: required", ERROR, 0, last_order);

					if (!HERE.empty())			environment_add(environment, "HERE", HERE);
					if (!GROUP_NAME.empty())	environment_add(environment, "GROUP_NAME", GROUP_NAME);
				}
			}
		}

	#pragma endregion

	#pragma region "Unix Server"

		void ConfigParser::validate_unix_server() {
			std::string	dir, sectionName = "unix_http_server";
			currentSection = sectionName;

			dir = get_value(sectionName, "directory");

			uint16_t	last_order = 0;
			std::string	filename;

			auto itSec = sections.find("unix_http_server");
			if (itSec != sections.end() && !itSec->second.empty()) {
				filename = itSec->second.begin()->second.filename;
				order = itSec->second.begin()->second.order;
			}

			std::string HERE = environment_get(environment, "HERE");

			environment_add(environment, "HERE", std::filesystem::path(filename).parent_path());

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

			ConfigEntry	*entry;
			entry = get_value_entry(sectionName, "file");
			if (!entry) error_add(filename, "[" + sectionName + "] file: required", ERROR, 0, last_order);

			if (!HERE.empty()) environment_add(environment, "HERE", HERE);
		}

	#pragma endregion

	#pragma region "Inet Server"

		void ConfigParser::validate_inet_server() {
			std::string	sectionName = "inet_http_server";
			currentSection = sectionName;

			uint16_t	last_order = 0;
			std::string	filename;

			auto itSec = sections.find("inet_http_server");
			if (itSec != sections.end() && !itSec->second.empty()) {
				filename = itSec->second.begin()->second.filename;
				order = itSec->second.begin()->second.order;
			}

			std::string HERE = environment_get(environment, "HERE");

			environment_add(environment, "HERE", std::filesystem::path(filename).parent_path());

			auto it = sections.find(sectionName);
			if (it != sections.end()) {
				for (auto &kv : it->second) {
					const std::string &key = kv.first;
					ConfigEntry &entry = kv.second;

					try { entry.value = environment_expand(environment, entry.value); }
					catch (const std::exception& e) {
						error_add(entry.filename, "[" + sectionName + "] " + key + ": unclosed quote or unfinished escape sequence", ERROR, entry.line, entry.order);
						if (key != "port") {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName][key];
						} else entry.value = "";
					}

					if (entry.value.empty() && key != "username" && key != "password") {
						error_add(entry.filename, "[" + sectionName + "] " + key + ": empty value", ERROR, entry.line, entry.order);
						if (key != "port") {
							error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName][key];
						}
					}

					if (key == "port" && !entry.value.empty() && !valid_port(entry.value)) {
						error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a valid TCP host:port", ERROR, entry.line, entry.order);
						entry.value = "";
					}

					if (key == "password" && !valid_password(entry.value)) {
						error_add(entry.filename, "[" + sectionName + "] " + key + ": invalid SHA format", ERROR, entry.line, entry.order);
						entry.value = "";
					}
				}
			}
			
			ConfigEntry	*entry;
			entry = get_value_entry(sectionName, "port");
			if (!entry) error_add(filename, "[" + sectionName + "] port: required", ERROR, 0, last_order);

			if (!HERE.empty()) environment_add(environment, "HERE", HERE);
		}

	#pragma endregion

	#pragma region "Options"

		int ConfigParser::validate_options(ConfigOptions& Options) const {
			static std::string	dir;
			std::string			errors;

			if (Options.options.find_first_of('d') != std::string::npos) {
				if (!valid_path(Options.directory, dir, true))
					errors += "directory:\t\tpath is invalid - " + std::string(strerror(errno)) + "\n";
				dir = expand_path(Options.directory, "", true, false);
			}

			if (Options.options.find_first_of('c') != std::string::npos) {
				if (!valid_path(Options.configuration, dir) || expand_path(Options.configuration, dir, true, false).empty())
					errors += "configuration:\t\tpath is invalid - " + std::string(strerror(errno)) + "\n";
			}

			if (Options.options.find_first_of('u') != std::string::npos) {
				if (!valid_user(Options.user))
					errors += "user:\t\t\tinvalid user\n";
			}

			if (Options.options.find_first_of('m') != std::string::npos) {
				if (!valid_umask(Options.umask))
					errors += "umask:\t\t\tmust be in octal format\n";
			}

			if (Options.options.find_first_of('l') != std::string::npos) {
				if (!valid_path(Options.logfile, dir, false, false, true) || expand_path(Options.logfile, dir).empty())
					errors += "logfile:\t\tpath is invalid - " + std::string(strerror(errno)) + "\n";
			}

			if (Options.options.find_first_of('y') != std::string::npos) {
				long bytes = parse_size(Options.logfile_maxbytes);
				if (bytes == -1 || !valid_number(std::to_string(bytes), 0, 1024 * 1024 * 1024))
					errors += "logfile_maxbytes:\tmust be a value between 0 bytes and 1024 MB\n";
			}

			if (Options.options.find_first_of('z') != std::string::npos) {
				if (!valid_number(Options.logfile_backups, 0, 1000))
					errors += "logfile_backups:\tmust be a value between 0 and 1000\n";
			}

			if (Options.options.find_first_of('e') != std::string::npos) {
				if (!valid_loglevel(Options.loglevel))
					errors += "loglevel:\t\tmust be one of: DEBUG, INFO, WARNING, ERROR, CRITICAL\n";
			}

			if (Options.options.find_first_of('j') != std::string::npos) {
				if (!valid_path(Options.pidfile, dir, false, false, true) || expand_path(Options.pidfile, dir).empty())
					errors += "pidfile:\t\tpath is invalid - " + std::string(strerror(errno)) + "\n";
			}

			if (Options.options.find_first_of('q') != std::string::npos) {
				if (!valid_path(Options.childlogdir, dir, true))
					errors += "childlogdir:\t\tpath is invalid - " + std::string(strerror(errno)) + "\n";
			}

			if (Options.options.find_first_of('a') != std::string::npos) {
				if (!valid_number(Options.minfds, 1, 65535))
					errors += "minfds:\t\t\tmust be a value between 1 and 65535\n";
			}

			if (Options.options.find_first_of('p') != std::string::npos) {
				if (!valid_number(Options.minprocs, 1, 65535))
					errors += "minprocs:\t\tmust be a value between 1 and 10000\n";
			}

			if (!errors.empty()) { std::cerr << Options.fullName << ": invalid options: \n\n" <<  errors; return (2); }

			return (0);
		}

	#pragma endregion

#pragma endregion

#pragma region "Validate"

	void ConfigParser::validate() {
		environment_initialize(environment);
		std::string HOST_NAME = environment_get(environment, "HOST_NAME");

		char hostname[255];
		environment_add(environment, "HOST_NAME", (!gethostname(hostname, sizeof(hostname))) ? std::string(hostname) : "unknown");

		validate_taskmasterd();
		if (has_section("unix_http_server")) validate_unix_server();
		if (has_section("inet_http_server")) validate_inet_server();
		validate_program();
		validate_group();

		if (!HOST_NAME.empty())	environment_add(environment, "HOST_NAME", HOST_NAME);
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
