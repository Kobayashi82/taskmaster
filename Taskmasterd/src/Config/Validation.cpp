/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Validation.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 11:32:25 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/04 18:33:34 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Utils/Utils.hpp"
	#include "Config/Config.hpp"
	#include "Logging/TaskmasterLog.hpp"

	#include <unistd.h>															// getuid()
	#include <cstring>															// strerror()
	#include <unistd.h>															// gethostname()
	#include <iostream>															// std::cerr
	#include <filesystem>														// std::filesystem::path()
	#include <pwd.h>															// struct passwd, getpwuid()

#pragma endregion

#pragma region "Validators"

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

					std::string HERE = Utils::environment_get(Utils::environment, "HERE");
					Utils::environment_add(Utils::environment, "HERE", std::filesystem::path(filename).parent_path());

					std::string numprocs = "1";
					entry = get_value_entry(sectionName, "numprocs");
					if (entry) {
						try { entry->value = Utils::environment_expand(Utils::environment, entry->value); }
						catch (const std::exception& e) {
							Utils::error_add(entry->filename, "[" + sectionName + "] numprocs: unclosed quote or unfinished escape sequence", ERROR, entry->line, entry->order);
							Utils::error_add(entry->filename, "[" + sectionName + "] numprocs: reset to default value: " + defaultValues[sectionName.substr(0, 8)]["numprocs"], WARNING, 0, entry->order + 1);
							entry->value = defaultValues[sectionName.substr(0, 8)]["numprocs"];
						}
						if (!Utils::valid_number(entry->value, 0, 100)) {
							Utils::error_add(entry->filename, "[" + sectionName + "] " + "numprocs: must be a value between 0 and 100", ERROR, entry->line, entry->order);
							Utils::error_add(entry->filename, "[" + sectionName + "] " + "numprocs: reset to default value: " + defaultValues[sectionName.substr(0, 8)]["numprocs"], WARNING, 0, entry->order + 1);
							entry->value = defaultValues[sectionName.substr(0, 8)]["numprocs"];
						}
					}

					std::string PROGRAM_NAME	= Utils::environment_get(Utils::environment, "PROGRAM_NAME");
					std::string NUMPROCS		= Utils::environment_get(Utils::environment, "NUMPROCS");

					Utils::environment_add(Utils::environment, "PROGRAM_NAME", program.substr(8));
					Utils::environment_add(Utils::environment, "NUMPROCS", numprocs);

					entry = get_value_entry(sectionName, "directory");
					if (entry) {
						try { entry->value = Utils::environment_expand(Utils::environment, entry->value); }
						catch (const std::exception& e) {
							Utils::error_add(entry->filename, "[" + sectionName + "] directory: unclosed quote or unfinished escape sequence", ERROR, entry->line, entry->order);
							Utils::error_add(entry->filename, "[" + sectionName + "] directory: reset to default value: " + dir, WARNING, 0, entry->order + 1);
							entry->value = dir;
						}

						std::string default_dir = dir;
						if (entry->value != "do not change") {
							if (!Utils::valid_path(entry->value, dir, true)) {
								Utils::error_add(entry->filename, "[" + sectionName + "] directory: invalid path - " + std::string(strerror(errno)), ERROR, entry->line, entry->order);
								if (!Utils::valid_path(default_dir, "", true))	Utils::error_add(entry->filename, "[" + sectionName + "] directory: failed to use default value - " + std::string(strerror(errno)), CRITICAL, 0, entry->order + 1);
								else {
									entry->value = Utils::expand_path(default_dir, dir, true, false);
									dir = entry->value;
									Utils::error_add(entry->filename, "[" + sectionName + "] directory: reset to default value: " + defaultValues[sectionName.substr(0, 8)]["directory"], WARNING, 0, entry->order + 1);
								}
							} else dir = Utils::expand_path(entry->value, dir, true, false);
						} else {
							if (!Utils::valid_path(default_dir, dir, true))	Utils::error_add(entry->filename, "[" + sectionName + "] directory: failed to use default value - " + std::string(strerror(errno)), CRITICAL, 0, entry->order + 1);
							else {
								entry->value = Utils::expand_path(default_dir, dir, true, false);
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
							if (key == "environment")	entry.value = Utils::environment_expand(Utils::environment, entry.value);
							else						entry.value = Utils::environment_expand(Utils::environment, entry.value);
						}
						catch (const std::exception& e) {
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": unclosed quote or unfinished escape sequence", ERROR, entry.line, entry.order);
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName.substr(0, 8)][key];
						}

						if (entry.value.empty() && key != "environment") {
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": empty value", ERROR, entry.line, entry.order);
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName.substr(0, 8)][key];
						}

						if (key == "tty_mode" && !Utils::valid_bool(entry.value)) {
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": must be true or false", ERROR, entry.line, entry.order);
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName.substr(0, 8)][key];
						}
						if (key == "autostart" && !Utils::valid_bool(entry.value)) {
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": must be true or false", ERROR, entry.line, entry.order);
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName.substr(0, 8)][key];
						}
						if (key == "stopasgroup" && !Utils::valid_bool(entry.value)) {
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": must be true or false", ERROR, entry.line, entry.order);
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName.substr(0, 8)][key];
						}
						if (key == "killasgroup" && !Utils::valid_bool(entry.value)) {
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": must be true or false", ERROR, entry.line, entry.order);
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName.substr(0, 8)][key];
						}
						if (key == "redirect_stderr" && !Utils::valid_bool(entry.value)) {
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": must be true or false", ERROR, entry.line, entry.order);
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName.substr(0, 8)][key];
						}
						if (key == "stdout_logfile_syslog" && !Utils::valid_bool(entry.value)) {
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": must be true or false", ERROR, entry.line, entry.order);
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName.substr(0, 8)][key];
						}
						if (key == "stderr_logfile_syslog" && !Utils::valid_bool(entry.value)) {
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": must be true or false", ERROR, entry.line, entry.order);
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName.substr(0, 8)][key];
						}

						if (key == "priority" && !Utils::valid_number(entry.value, 0, 999)) {
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a value between 0 and 999", ERROR, entry.line, entry.order);
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName.substr(0, 8)][key];
						}
						if (key == "startsecs" && !Utils::valid_number(entry.value, 0, 3600)) {
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a value between 0 and 3600", ERROR, entry.line, entry.order);
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName.substr(0, 8)][key];
						}
						if (key == "startretries" && !Utils::valid_number(entry.value, 0, 100)) {
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a value between 0 and 100", ERROR, entry.line, entry.order);
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName.substr(0, 8)][key];
						}
						if (key == "stopwaitsecs" && !Utils::valid_number(entry.value, 1, 3600)) {
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": mmust be a value between 1 and 3600", ERROR, entry.line, entry.order);
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName.substr(0, 8)][key];
						}

						if (key == "stdout_logfile_maxbytes") {
							long bytes = Utils::parse_size(entry.value);
							if (bytes == -1 || !Utils::valid_number(std::to_string(bytes), 0, 1024 * 1024 * 1024)) {
								Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a value between 0 bytes and 1024 MB", ERROR, entry.line, entry.order);
								Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
								entry.value = defaultValues[sectionName.substr(0, 8)][key];
							}
						}
						if (key == "stdout_logfile_backups" && !Utils::valid_number(entry.value, 0, 1000)) {
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a value between 0 and 1000", ERROR, entry.line, entry.order);
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName.substr(0, 8)][key];
						}

						if (key == "stderr_logfile_maxbytes") {
							long bytes = Utils::parse_size(entry.value);
							if (bytes == -1 || !Utils::valid_number(std::to_string(bytes), 0, 1024 * 1024 * 1024)) {
								Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a value between 0 bytes and 1024 MB", ERROR, entry.line, entry.order);
								Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
								entry.value = defaultValues[sectionName.substr(0, 8)][key];
							}
						}
						if (key == "stderr_logfile_backups" && !Utils::valid_number(entry.value, 0, 1000)) {
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a value between 0 and 1000", ERROR, entry.line, entry.order);
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName.substr(0, 8)][key];
						}

						if (key == "process_name") {
							std::string numprocs;
							try { numprocs =  Utils::environment_expand(Utils::environment, get_value(sectionName, "numprocs")); }
							catch (const std::exception& e) { numprocs = "0"; }
							if (numprocs != "1" && entry.value.find("${PROCESS_NUM:") == std::string::npos && entry.value.find("${PROCESS_NUM}") == std::string::npos && entry.value.find("$PROCESS_NUM") == std::string::npos
								&& entry.value.find("${EX_PROCESS_NUM:") == std::string::npos && entry.value.find("${EX_PROCESS_NUM}") == std::string::npos && entry.value.find("$EX_PROCESS_NUM") == std::string::npos) {
								Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": must include $PROCESS_NUM when 'numprocs' is greater than 1", ERROR, entry.line, entry.order);
							}
						}

						if (key == "numprocs") {
							if (!Utils::valid_number(entry.value, 1, 1000))
								Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a value between 1 and 1000", ERROR, entry.line, entry.order);
							if (entry.value != "1") {
								std::string process_name;
								try { process_name = Utils::environment_expand(Utils::environment, get_value(sectionName, "process_name")); }
								catch (const std::exception& e) { process_name = ""; }
								if (process_name.empty() || (process_name.find("${PROCESS_NUM:") == std::string::npos && process_name.find("${PROCESS_NUM}") == std::string::npos && process_name.find("$PROCESS_NUM") == std::string::npos
									&& process_name.find("${EX_PROCESS_NUM:") == std::string::npos && process_name.find("${EX_PROCESS_NUM}") == std::string::npos && process_name.find("$EX_PROCESS_NUM") == std::string::npos))
									Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": 'process_name' must include $PROCESS_NUM when 'numprocs' is greater than 1", ERROR, entry.line, entry.order);
							}
						}

						if (key == "numprocs_start" && !Utils::valid_number(entry.value, 0, 65535)) {
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a value between 0 and 65535", ERROR, entry.line, entry.order);
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName.substr(0, 8)][key];
						}

						if (key == "autorestart" && !Utils::valid_autorestart(entry.value)) {
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": must be false, unexpected or true", ERROR, entry.line, entry.order);
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName.substr(0, 8)][key];
						}

						if (key == "exitcodes" && !Utils::valid_code(entry.value)) {
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": must be comma-separated numbers between 0 and 255", ERROR, entry.line, entry.order);
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName.substr(0, 8)][key];
						}

						if (key == "stopsignal" && !Utils::valid_signal(entry.value)) {
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a valid signal (HUP, INT, QUIT, KILL, TERM, USR1, USR2)", ERROR, entry.line, entry.order);
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName.substr(0, 8)][key];
						}

						if (key == "user") {
							if (!Utils::valid_user(entry.value)) {
								Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": invalid user", CRITICAL, entry.line, entry.order);
								entry.value = "";
							} else {
								if (!is_root && Utils::toLower(entry.value) != "do not switch") {
									struct passwd* pw = getpwuid(getuid());
									if (pw && pw->pw_name != entry.value && entry.value != std::to_string(getuid()))
										Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": cannot switch user, not running as root", CRITICAL, entry.line, entry.order);
								}
							}
						}

						if (key == "umask") {
							if (entry.value == "inherit") entry.value = defaultValues["taskmasterd"][key];
							if (!Utils::valid_umask(entry.value)) {
								Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": must be in octal format", ERROR, entry.line, entry.order);
								Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
								entry.value = defaultValues[sectionName.substr(0, 8)][key];
							}
						}

						if (key == "stdout_logfile") {
							if (Utils::toUpper(entry.value) == "AUTO") {
								std::string childlogdir = get_value("taskmasterd", "childlogdir");
								entry.value = sectionName.substr(8) + "_stdout.log";
								if (!Utils::valid_path(entry.value, childlogdir)) {
									Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": invalid path - " + std::string(strerror(errno)), ERROR, entry.line, entry.order);
									entry.value = "NONE";
								}
							} else if (Utils::toUpper(entry.value) != "NONE") {
								if (!Utils::valid_path(entry.value, dir)) {
									Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": invalid path - " + std::string(strerror(errno)), ERROR, entry.line, entry.order);
									if (entry.value != defaultValues[sectionName.substr(0, 8)][key]) {
										Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
										entry.value = defaultValues[sectionName.substr(0, 8)][key];
										if (!Utils::valid_path(entry.value, dir)) {
											Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": failed to use default value - " + std::string(strerror(errno)), ERROR, entry.line, entry.order);
											entry.value = "NONE";
										}
									} else entry.value = "NONE";
								}
							}
							if (Utils::toUpper(entry.value) != "NONE") entry.value = Utils::expand_path(entry.value, dir);
						}

						if (key == "stderr_logfile") {
							if (Utils::toUpper(entry.value) == "AUTO") {
								std::string childlogdir = get_value("taskmasterd", "childlogdir");
								entry.value = sectionName.substr(8) + "_stdout.log";
								if (!Utils::valid_path(entry.value, childlogdir)) {
									Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": invalid path - " + std::string(strerror(errno)), ERROR, entry.line, entry.order);
									entry.value = "NONE";
								}
							} else if (Utils::toUpper(entry.value) != "NONE") {
								if (!Utils::valid_path(entry.value, dir)) {
									Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": invalid path - " + std::string(strerror(errno)), ERROR, entry.line, entry.order);
									if (entry.value != defaultValues[sectionName.substr(0, 8)][key]) {
										Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
										entry.value = defaultValues[sectionName.substr(0, 8)][key];
										if (!Utils::valid_path(entry.value, dir)) {
											Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": failed to use default value - " + std::string(strerror(errno)), ERROR, entry.line, entry.order);
											entry.value = "NONE";
										}
									} else entry.value = "NONE";
								}
							}
							if (Utils::toUpper(entry.value) != "NONE") entry.value = Utils::expand_path(entry.value, dir);
						}

						if (key == "serverurl") {
							if (Utils::toUpper(entry.value) != "AUTO" && !Utils::valid_serverurl(entry.value)) {
								Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": invalid format", ERROR, entry.line, entry.order);
								Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 8)][key], WARNING, 0, entry.order + 1);
								entry.value = defaultValues[sectionName.substr(0, 8)][key];
							}
							if (Utils::toUpper(entry.value) == "AUTO") {
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

						if (key == "environment" && !Utils::environment_validate(entry.value)) {
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": invalid variable format", ERROR, entry.line, entry.order);
							entry.value = "";
						}
					}

					entry = get_value_entry(sectionName, "command");
					if (entry) {
						try { entry->value = Utils::environment_expand(Utils::environment, entry->value); }
						catch (const std::exception& e) { Utils::error_add(entry->filename, "[" + sectionName + "] command: unclosed quote or unfinished escape sequence", ERROR, entry->line, entry->order); }

						if (entry->value.empty()) {
							Utils::error_add(entry->filename, "[" + sectionName + "] command: empty value", ERROR, entry->line, entry->order);
						} else if ((entry->value = Utils::parse_executable(entry->value)).empty()) {
							Utils::error_add(entry->filename, "[" + sectionName + "] command: must be a valid executable", ERROR, entry->line, entry->order);
						}
					} else Utils::error_add(filename, "[" + sectionName + "] command: required", ERROR, 0, last_order);

					if (!HERE.empty())			Utils::environment_add(Utils::environment, "HERE", HERE);
					if (!PROGRAM_NAME.empty())	Utils::environment_add(Utils::environment, "PROGRAM_NAME", PROGRAM_NAME);
					if (!NUMPROCS.empty())		Utils::environment_add(Utils::environment, "NUMPROCS", NUMPROCS);
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

					std::string HERE		= Utils::environment_get(Utils::environment, "HERE");
					std::string GROUP_NAME	= Utils::environment_get(Utils::environment, "GROUP_NAME");

					Utils::environment_add(Utils::environment, "HERE", std::filesystem::path(filename).parent_path());
					Utils::environment_add(Utils::environment, "GROUP_NAME", group.substr(6));

					for (auto &kv : keys) {
						const std::string &key = kv.first;
						ConfigEntry &entry = kv.second;

						try { entry.value = Utils::environment_expand(Utils::environment, entry.value); }
						catch (const std::exception& e) {
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": unclosed quote or unfinished escape sequence", ERROR, entry.line, entry.order);
							if (key != "programs") {
								Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 6)][key], WARNING, 0, entry.order + 1);
								entry.value = defaultValues[sectionName.substr(0, 6)][key];
							} else entry.value = "";
						}

						if (entry.value.empty()) {
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": empty value", ERROR, entry.line, entry.order);
							if (key != "programs") {
								Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 6)][key], WARNING, 0, entry.order + 1);
								entry.value = defaultValues[sectionName.substr(0, 6)][key];
							}
						}

						if (key == "programs" && !entry.value.empty()) {
							std::string name;
							std::istringstream cm_stream(entry.value);
							while (std::getline(cm_stream, name, ',')) {
								if (!programs.count(Utils::trim(name))) Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": " + Utils::trim(name) + " does not exist or is not configured correctly", ERROR, entry.line, entry.order);
							}
						}

						if (key == "priority" && !Utils::valid_number(entry.value, 0, 999)) {
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a value between 0 and 999", ERROR, entry.line, entry.order);
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName.substr(0, 6)][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName.substr(0, 6)][key];
						}
					}

					ConfigEntry	*entry;
					entry = get_value_entry(sectionName, "programs");
					if (!entry) Utils::error_add(filename, "[" + sectionName + "] programs: required", ERROR, 0, last_order);

					if (!HERE.empty())			Utils::environment_add(Utils::environment, "HERE", HERE);
					if (!GROUP_NAME.empty())	Utils::environment_add(Utils::environment, "GROUP_NAME", GROUP_NAME);
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

			std::string HERE = Utils::environment_get(Utils::environment, "HERE");

			Utils::environment_add(Utils::environment, "HERE", std::filesystem::path(filename).parent_path());

			auto it = sections.find(sectionName);
			if (it != sections.end()) {
				for (auto &kv : it->second) {
					const std::string &key = kv.first;
					ConfigEntry &entry = kv.second;

					try { entry.value = Utils::environment_expand(Utils::environment, entry.value); }
					catch (const std::exception& e) {
						Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": unclosed quote or unfinished escape sequence", ERROR, entry.line, entry.order);
						Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
						entry.value = defaultValues[sectionName][key];
					}

					if (entry.value.empty() && key != "username" && key != "password") {
						Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": empty value", ERROR, entry.line, entry.order);
						Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
						entry.value = defaultValues[sectionName][key];
					}

					if (key == "file") {
						if (!Utils::valid_path(entry.value, dir)) {
							if (entry.value != defaultValues[sectionName][key]) {
								Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": invalid path - " + std::string(strerror(errno)), ERROR, entry.line, entry.order);
								Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
								entry.value = defaultValues[sectionName][key];
								if (!Utils::valid_path(entry.value, dir)) {
									Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": failed to use default value - " + std::string(strerror(errno)), ERROR, entry.line, entry.order);
									entry.value = "NONE";
								}
							} else {
								Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": invalid path - " + std::string(strerror(errno)), ERROR, entry.line, entry.order);
								entry.value = "NONE";
							}
						}
						if (Utils::toUpper(entry.value) != "NONE") entry.value = Utils::expand_path(entry.value, dir);
					}

					if (key == "chmod" && !Utils::valid_umask(entry.value)) {
						Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": must be in octal format", ERROR, entry.line, entry.order);
						Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
						entry.value = defaultValues[sectionName][key];
					}

					if (key == "chown" && !Utils::valid_chown(entry.value)) {
						Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": invalid user or group", ERROR, entry.line, entry.order);
						entry.value = "";
					}

					if (key == "password" && !Utils::valid_password(entry.value)) {
						Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": invalid SHA format", ERROR, entry.line, entry.order);
						entry.value = "";
					}
				}
			}

			ConfigEntry	*entry;
			entry = get_value_entry(sectionName, "file");
			if (!entry) Utils::error_add(filename, "[" + sectionName + "] file: required", ERROR, 0, last_order);

			if (!HERE.empty()) Utils::environment_add(Utils::environment, "HERE", HERE);
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

			std::string HERE = Utils::environment_get(Utils::environment, "HERE");

			Utils::environment_add(Utils::environment, "HERE", std::filesystem::path(filename).parent_path());

			auto it = sections.find(sectionName);
			if (it != sections.end()) {
				for (auto &kv : it->second) {
					const std::string &key = kv.first;
					ConfigEntry &entry = kv.second;

					try { entry.value = Utils::environment_expand(Utils::environment, entry.value); }
					catch (const std::exception& e) {
						Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": unclosed quote or unfinished escape sequence", ERROR, entry.line, entry.order);
						if (key != "port") {
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName][key];
						} else entry.value = "";
					}

					if (entry.value.empty() && key != "username" && key != "password") {
						Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": empty value", ERROR, entry.line, entry.order);
						if (key != "port") {
							Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": reset to default value: " + defaultValues[sectionName][key], WARNING, 0, entry.order + 1);
							entry.value = defaultValues[sectionName][key];
						}
					}

					if (key == "port" && !entry.value.empty() && !Utils::valid_port(entry.value)) {
						Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": must be a valid TCP host:port", ERROR, entry.line, entry.order);
						entry.value = "";
					}

					if (key == "password" && !Utils::valid_password(entry.value)) {
						Utils::error_add(entry.filename, "[" + sectionName + "] " + key + ": invalid SHA format", ERROR, entry.line, entry.order);
						entry.value = "";
					}
				}
			}
			
			ConfigEntry	*entry;
			entry = get_value_entry(sectionName, "port");
			if (!entry) Utils::error_add(filename, "[" + sectionName + "] port: required", ERROR, 0, last_order);

			if (!HERE.empty()) Utils::environment_add(Utils::environment, "HERE", HERE);
		}

	#pragma endregion

	#pragma region "Options"

		int ConfigParser::validate_options(ConfigOptions& Options) const {
			static std::string	dir;
			std::string			errors;

			if (Options.options.find_first_of('d') != std::string::npos) {
				if (!Utils::valid_path(Options.directory, dir, true))
					errors += "directory:\t\tpath is invalid - " + std::string(strerror(errno)) + "\n";
				dir = Utils::expand_path(Options.directory, "", true, false);
			}

			if (Options.options.find_first_of('c') != std::string::npos) {
				if (!Utils::valid_path(Options.configuration, dir) || Utils::expand_path(Options.configuration, dir, true, false).empty())
					errors += "configuration:\t\tpath is invalid - " + std::string(strerror(errno)) + "\n";
			}

			if (Options.options.find_first_of('u') != std::string::npos) {
				if (!Utils::valid_user(Options.user))
					errors += "user:\t\t\tinvalid user\n";
			}

			if (Options.options.find_first_of('m') != std::string::npos) {
				if (!Utils::valid_umask(Options.umask))
					errors += "umask:\t\t\tmust be in octal format\n";
			}

			if (Options.options.find_first_of('l') != std::string::npos) {
				if (!Utils::valid_path(Options.logfile, dir, false, false, true) || Utils::expand_path(Options.logfile, dir).empty())
					errors += "logfile:\t\tpath is invalid - " + std::string(strerror(errno)) + "\n";
			}

			if (Options.options.find_first_of('y') != std::string::npos) {
				long bytes = Utils::parse_size(Options.logfile_maxbytes);
				if (bytes == -1 || !Utils::valid_number(std::to_string(bytes), 0, 1024 * 1024 * 1024))
					errors += "logfile_maxbytes:\tmust be a value between 0 bytes and 1024 MB\n";
			}

			if (Options.options.find_first_of('z') != std::string::npos) {
				if (!Utils::valid_number(Options.logfile_backups, 0, 1000))
					errors += "logfile_backups:\tmust be a value between 0 and 1000\n";
			}

			if (Options.options.find_first_of('e') != std::string::npos) {
				if (!Utils::valid_loglevel(Options.loglevel))
					errors += "loglevel:\t\tmust be one of: DEBUG, INFO, WARNING, ERROR, CRITICAL\n";
			}

			if (Options.options.find_first_of('j') != std::string::npos) {
				if (!Utils::valid_path(Options.pidfile, dir, false, false, true) || Utils::expand_path(Options.pidfile, dir).empty())
					errors += "pidfile:\t\tpath is invalid - " + std::string(strerror(errno)) + "\n";
			}

			if (Options.options.find_first_of('q') != std::string::npos) {
				if (!Utils::valid_path(Options.childlogdir, dir, true))
					errors += "childlogdir:\t\tpath is invalid - " + std::string(strerror(errno)) + "\n";
			}

			if (Options.options.find_first_of('a') != std::string::npos) {
				if (!Utils::valid_number(Options.minfds, 1, 65535))
					errors += "minfds:\t\t\tmust be a value between 1 and 65535\n";
			}

			if (Options.options.find_first_of('p') != std::string::npos) {
				if (!Utils::valid_number(Options.minprocs, 1, 65535))
					errors += "minprocs:\t\tmust be a value between 1 and 10000\n";
			}

			if (!errors.empty()) { std::cerr << Options.fullName << ": invalid options: \n\n" <<  errors; return (2); }

			return (0);
		}

	#pragma endregion

#pragma endregion

#pragma region "Validate"

	void ConfigParser::validate() {
		Utils::environment_initialize(Utils::environment);
		std::string HOST_NAME = Utils::environment_get(Utils::environment, "HOST_NAME");

		char hostname[255];
		Utils::environment_add(Utils::environment, "HOST_NAME", (!gethostname(hostname, sizeof(hostname))) ? std::string(hostname) : "unknown");

		if (has_section("unix_http_server")) validate_unix_server();
		if (has_section("inet_http_server")) validate_inet_server();
		validate_program();
		validate_group();

		if (!HOST_NAME.empty())	Utils::environment_add(Utils::environment, "HOST_NAME", HOST_NAME);
	}

#pragma endregion
