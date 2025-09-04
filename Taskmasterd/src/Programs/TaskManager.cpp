/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TaskManager.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/02 17:23:05 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/04 23:42:04 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Utils/Utils.hpp"
	#include "Programs/TaskManager.hpp"
	#include "Logging/TaskmasterLog.hpp"

	#include <cstring>															// strerror()
	#include <unistd.h>															// gethostname()
	#include <filesystem>														// std::filesistem::path()
	#include <pwd.h>															// struct passwd, getpwuid()
	#include <algorithm>
	#include <iostream>

#pragma endregion

#pragma region "Variables"

	TaskManager TaskMaster;

#pragma endregion

#pragma region "Validate"

	std::string TaskManager::validate(const std::string& key, ConfigParser::ConfigEntry *entry) {
		if (key.empty() || !entry) return {};

		static std::string			dir = "";
		std::string					sectionName = "taskmasterd";
		std::vector<std::string>	bool_values = { "nodaemon", "silent", "logfile_syslog", "strip_ansi", "nocleanup" };

		if (key == "directory") {
			std::string default_dir = Utils::expand_path(".", "", true, false);
			if (entry->value != "do not change") {
				if (!Utils::valid_path(entry->value, "", true)) {
					Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": invalid path - " + std::string(strerror(errno)), ERROR, entry->line, entry->order);
					if (!Utils::valid_path(default_dir, "", true)) {
						Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": failed to use default value - " + std::string(strerror(errno)), CRITICAL, 0, entry->order + 1);
						entry->value = "";
					}else {
						entry->value = Utils::expand_path(default_dir, "", true, false);
						Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": reset to default value: " + ((default_dir.empty()) ? "current directory" : default_dir), WARNING, 0, entry->order + 1);
					}
				} else entry->value = Utils::expand_path(entry->value, "", true, false);
			} else {
				if (!Utils::valid_path(default_dir, "", true)) {
					Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": failed to use default value - " + std::string(strerror(errno)), CRITICAL, 0, entry->order + 1);
					entry->value = "";
				}
				else entry->value = Utils::expand_path(default_dir, "", true, false);
			}
			dir = entry->value;
		}

		if (std::find(bool_values.begin(), bool_values.end(), key) != bool_values.end() && !Utils::valid_bool(entry->value)) {
			std::cerr << key << " - " << entry->value << "\n";
			Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": must be true or false", ERROR, entry->line, entry->order);
			Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": reset to default value: " + Config.defaultValues[sectionName][key], WARNING, 0, entry->order + 1);
			entry->value = Config.defaultValues[sectionName][key];
		}

		if (key == "user") {
			if (!Utils::valid_user(entry->value)) {
				Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": invalid user", CRITICAL, entry->line, entry->order);
				entry->value = "";
			} else {
				if (!Config.is_root && Utils::toLower(entry->value) != "do not switch") {
					struct passwd* pw = getpwuid(getuid());
					if (pw && pw->pw_name != entry->value && entry->value != std::to_string(getuid()))
						Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": cannot drop privileges, not running as root", CRITICAL, entry->line, entry->order);
				}
			}
		}

		if (key == "umask" && !Utils::valid_umask(entry->value)) {
			Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": must be in octal format", ERROR, entry->line, entry->order);
			Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": reset to default value: " + Config.defaultValues[sectionName][key], WARNING, 0, entry->order + 1);
			entry->value = Config.defaultValues[sectionName][key];
		}

		if (key == "logfile") {
			if (Utils::toUpper(entry->value) != "NONE") {
				if (!Utils::valid_path(entry->value, dir, false)) {
					Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": invalid path - " + std::string(strerror(errno)), ERROR, entry->line, entry->order);
					if (entry->value != Config.defaultValues[sectionName][key]) {
						Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": reset to default value: " + Config.defaultValues[sectionName][key], WARNING, 0, entry->order + 1);
						entry->value = Config.defaultValues[sectionName][key];
						if (!Utils::valid_path(entry->value, dir)) {
							Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": failed to use default value - " + std::string(strerror(errno)), ERROR, entry->line, entry->order);
							entry->value = "NONE";
						}
					} else entry->value = "NONE";
				}
			}
			if (Utils::toUpper(entry->value) != "NONE") entry->value = Utils::expand_path(entry->value, dir);
		}

		if (key == "logfile_maxbytes") {
			long bytes = Utils::parse_size(entry->value);
			if (bytes == -1 || !Utils::valid_number(std::to_string(bytes), 0, 1024 * 1024 * 1024)) {
				Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": must be a value between 0 bytes and 1024 MB", ERROR, entry->line, entry->order);
				Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": reset to default value: " + Config.defaultValues[sectionName][key], WARNING, 0, entry->order + 1);
				entry->value = Config.defaultValues[sectionName][key];
			}
		}
		if (key == "logfile_backups" && !Utils::valid_number(entry->value, 0, 1000)) {
			Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": must be a value between 0 and 1000", ERROR, entry->line, entry->order);
			Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": reset to default value: " + Config.defaultValues[sectionName][key], WARNING, 0, entry->order + 1);
			entry->value = Config.defaultValues[sectionName][key];
		}

		if (key == "loglevel" && !Utils::valid_loglevel(entry->value)) {
			Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": must be one of: DEBUG, INFO, WARNING, ERROR, CRITICAL", ERROR, entry->line, entry->order);
			Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": reset to default value: " + Config.defaultValues[sectionName][key], WARNING, 0, entry->order + 1);
			entry->value = Config.defaultValues[sectionName][key];
		}

		if (key == "pidfile") {
			if (!Utils::valid_path(entry->value, dir)) {
				if (entry->value != Config.defaultValues[sectionName][key]) {
					Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": invalid path - " + std::string(strerror(errno)), ERROR, entry->line, entry->order);
					Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": reset to default value: " + Config.defaultValues[sectionName][key], WARNING, 0, entry->order + 1);
					entry->value = Config.defaultValues[sectionName][key];
					if (!Utils::valid_path(entry->value, dir)) {
						Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": failed to use default value - " + std::string(strerror(errno)), CRITICAL, entry->line, entry->order);
					}
				} else Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": invalid path - " + std::string(strerror(errno)), CRITICAL, entry->line, entry->order);
			}
			entry->value = Utils::expand_path(entry->value, dir);
		}

		if (key == "childlogdir") {
			if (!Utils::valid_path(entry->value, dir, true)) {
				if (entry->value != Config.defaultValues[sectionName][key]) {
					Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": invalid path - " + std::string(strerror(errno)), ERROR, entry->line, entry->order);
					Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": reset to default value: " + Config.defaultValues[sectionName][key], WARNING, 0, entry->order + 1);
					entry->value = Config.defaultValues[sectionName][key];
					if (!Utils::valid_path(entry->value, dir)) {
						Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": failed to use default value - " + std::string(strerror(errno)), ERROR, entry->line, entry->order);
						entry->value = "NONE";
					}
				} else {
					Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": invalid path - " + std::string(strerror(errno)), ERROR, entry->line, entry->order);
					entry->value = "NONE";
				}
			}
			if (entry->value != "NONE") entry->value = Utils::expand_path(entry->value, dir, true, false);
		}

		if (key == "minfds") {
			if (!Utils::valid_number(entry->value, 1, 65535)) {
				Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": must be a value between 1 and 65535", ERROR, entry->line, entry->order);
				Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": reset to default value: " + Config.defaultValues[sectionName][key], WARNING, 0, entry->order + 1);
				entry->value = Config.defaultValues[sectionName][key];
			}
			if (Utils::parse_fd_limit(static_cast<uint16_t>(std::stoul(entry->value)))) {
				if (std::stoul(entry->value) > std::stoul(Config.defaultValues[sectionName][key])) {
					Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": limit could not be applied - system limit too low or insufficient permissions", ERROR, entry->line, entry->order);
					Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": reset to default value: " + Config.defaultValues[sectionName][key], WARNING, 0, entry->order + 1);
					entry->value = Config.defaultValues[sectionName][key];
					if (Utils::parse_fd_limit(static_cast<uint16_t>(std::stoul(entry->value)))) {
						Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": limit could not be applied - system limit too low or insufficient permissions", CRITICAL, entry->line, entry->order);
					}
				} else Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": limit could not be applied - system limit too low or insufficient permissions", CRITICAL, entry->line, entry->order);
			}
		}

		if (key == "minprocs") {
			if (!Utils::valid_number(entry->value, 1, 10000)) {
				Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": must be a value between 1 and 10000", ERROR, entry->line, entry->order);
				Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": reset to default value: " + Config.defaultValues[sectionName][key], WARNING, 0, entry->order + 1);
				entry->value = Config.defaultValues[sectionName][key];
			}
			if (Utils::parse_process_limit(static_cast<uint16_t>(std::stoul(entry->value)))) {
				if (std::stoul(entry->value) > std::stoul(Config.defaultValues[sectionName][key])) {
					Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": limit could not be applied - system limit too low or insufficient permissions", ERROR, entry->line, entry->order);
					Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": reset to default value: " + Config.defaultValues[sectionName][key], WARNING, 0, entry->order + 1);
					entry->value = Config.defaultValues[sectionName][key];
					if (Utils::parse_process_limit(static_cast<uint16_t>(std::stoul(entry->value)))) {
						Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": limit could not be applied - system limit too low or insufficient permissions", CRITICAL, entry->line, entry->order);
					}
				} else Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": limit could not be applied - system limit too low or insufficient permissions", CRITICAL, entry->line, entry->order);
			}
		}

		return (entry->value);
	}

#pragma endregion

#pragma region "Expand Vars (cambiar)"

	std::string TaskManager::expand_vars(std::map<std::string, std::string>& env, const std::string& key) {
		if (key.empty()) return {};

		std::string sectionName = "taskmasterd";

		ConfigParser::ConfigEntry *entry = Config.get_value_entry(sectionName, key);
		if (!entry) return {};

		try {
			entry->value = Utils::environment_expand(env, entry->value);
			if (key == "environment") {
				if (!Utils::environment_validate(entry->value)) {
					Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": invalid variable format", ERROR, entry->line, entry->order);
					entry->value = "";
				} else Utils::environment_add_batch(env, entry->value);
			} else entry->value = Utils::remove_quotes(entry->value);
		}
		catch(const std::exception& e) {
			Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": unclosed quote or unfinished escape sequence", ERROR, entry->line, entry->order);
			if (key != "environment") {
				Utils::error_add(entry->filename, "[" + sectionName + "] " + key + ": reset to default value: " + Config.defaultValues[sectionName][key], WARNING, 0, entry->order + 1);
				entry->value = Config.defaultValues[sectionName][key];
			} else entry->value = "";
		}

		return (validate(key, entry));
	}

#pragma endregion

#pragma region "Initialize"

	void TaskManager::initialize() {
		std::string configFile;
		ConfigParser::ConfigEntry *entry = Config.get_value_entry("taskmasterd", "directory");
		if (!entry)	configFile = Utils::expand_path(".", "", true, false);
		else		configFile = entry->filename;

		Utils::environment_initialize(environment);

		std::string HERE			= Utils::environment_get(environment, "HERE");
		std::string HOST_NAME		= Utils::environment_get(environment, "HOST_NAME");

		char hostname[255];
		Utils::environment_add(environment, "HOST_NAME", (!gethostname(hostname, sizeof(hostname))) ? std::string(hostname) : "unknown");
		if (!configFile.empty()) Utils::environment_add(environment, "HERE", std::filesystem::path(configFile).parent_path());

		directory			= expand_vars(environment, "directory");
		nodaemon			= Utils::parse_bool(expand_vars(environment, "nodaemon"));
		silent				= Utils::parse_bool(expand_vars(environment, "silent"));
		user				= expand_vars(environment, "user");
		umask				= static_cast<uint16_t>(std::stoi(expand_vars(environment, "umask"), nullptr, 8));
		logfile				= expand_vars(environment, "logfile");
		logfile_maxbytes	= Utils::parse_size(expand_vars(environment, "logfile_maxbytes"));
		logfile_backups		= Utils::parse_number(expand_vars(environment, "logfile_backups"), 0, 1000, 10);
		logfile_syslog		= Utils::parse_bool(expand_vars(environment, "logfile_syslog"));
		loglevel			= Utils::parse_loglevel(expand_vars(environment, "loglevel"));
		pidfile				= expand_vars(environment, "pidfile");
		childlogdir			= expand_vars(environment, "childlogdir");
		strip_ansi			= Utils::parse_bool(expand_vars(environment, "strip_ansi"));
		nocleanup			= Utils::parse_bool(expand_vars(environment, "nocleanup"));
		minfds				= Utils::parse_number(expand_vars(environment, "minfds"), 1, 65535, 1024);
		minprocs			= Utils::parse_number(expand_vars(environment, "minprocs"), 1, 10000, 200);
		
		if (HERE.empty())		Utils::environment_del(environment, "HERE");
		else					Utils::environment_add(environment, "HERE", HERE);
		if (HOST_NAME.empty())	Utils::environment_del(environment, "HOST_NAME");
		else					Utils::environment_add(environment, "HOST_NAME", HOST_NAME);

		expand_vars(environment, "environment");

		for (auto& [program, keys] : Config.sections) {
			if (program.substr(0, 8) == "program:") {
				programs.emplace_back(program);
			}
		}

		for (auto& [group, keys] : Config.sections) {
			if (group.substr(0, 6) == "group:") {
				if (Config.get_value(group, "programs").empty()) continue;
				groups.emplace_back(group);
			}
		}
	}

#pragma endregion
