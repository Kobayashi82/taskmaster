/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Validation.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 11:32:25 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/04 23:21:53 by vzurera-         ###   ########.fr       */
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
		validate_group();

		if (!HOST_NAME.empty())	Utils::environment_add(Utils::environment, "HOST_NAME", HOST_NAME);
	}

#pragma endregion
