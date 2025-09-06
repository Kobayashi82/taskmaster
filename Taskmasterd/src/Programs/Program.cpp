/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Program.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/02 17:23:05 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/06 18:42:22 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Utils/Utils.hpp"
	#include "Config/Config.hpp"
	#include "Programs/Program.hpp"
	#include "Programs/TaskManager.hpp"

	#include <cstring>															// strerror()
	#include <unistd.h>															// gethostname()
	#include <filesystem>														// std::filesistem::path()
	#include <pwd.h>															// struct passwd, getpwuid()
	#include <algorithm>
	#include <iostream>
	#include <sstream>															// std::stringstream

#pragma endregion

#pragma region "Constructors"

	Program::Program(const std::string _section) : section(_section), name(_section.substr(8)) {
		initialize();
	}

#pragma endregion

#pragma region "Validate"

	std::string Program::validate(const std::string& key, ConfigParser::ConfigEntry *entry) {
		if (key.empty() || !entry) return {};

		static std::string			dir = "";
		std::vector<std::string>	bool_values = { "tty_mode", "autostart", "stopasgroup", "killasgroup", "redirect_stderr", "stdout_logfile_syslog", "stderr_logfile_syslog" };

		if (key == "directory") {
			std::string default_dir = TaskMaster.directory;
			if (entry->value != "do not change") {
				if (!Utils::valid_path(entry->value, "", true)) {
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": invalid path - " + std::string(strerror(errno)), ERROR, entry->line, entry->order);
					if (!Utils::valid_path(default_dir, "", true)) {
						Utils::error_add(entry->filename, "[" + section + "] " + key + ": failed to use default value - " + std::string(strerror(errno)), ERROR, 0, entry->order + 1);
						entry->value = ""; disabled = true;
					}else {
						entry->value = Utils::expand_path(default_dir, "", true, false);
						Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + ((default_dir.empty()) ? "current directory" : default_dir), WARNING, 0, entry->order + 1);
					}
				} else entry->value = Utils::expand_path(entry->value, "", true, false);
			} else {
				if (!Utils::valid_path(default_dir, "", true)) {
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": failed to use default value - " + std::string(strerror(errno)), ERROR, 0, entry->order + 1);
					entry->value = ""; disabled = true;
				}
				else entry->value = Utils::expand_path(default_dir, "", true, false);
			}
			dir = entry->value;
		}

		if (key == "numprocs" && !Utils::valid_number(entry->value, 1, 10000)) {
			Utils::error_add(entry->filename, "[" + section + "] " + key + ": must be a value between 1 and 10000", ERROR, entry->line, entry->order);
			Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section][key], WARNING, 0, entry->order + 1);
			entry->value = Config.defaultValues[section][key];
		}
		if (key == "numprocs_start" && !Utils::valid_number(entry->value, 0, 65535)) {
			Utils::error_add(entry->filename, "[" + section + "] " + key + ": must be a value between 0 and 65535", ERROR, entry->line, entry->order);
			Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section][key], WARNING, 0, entry->order + 1);
			entry->value = Config.defaultValues[section][key];
		}

		if (std::find(bool_values.begin(), bool_values.end(), key) != bool_values.end() && !Utils::valid_bool(entry->value)) {
			Utils::error_add(entry->filename, "[" + section + "] " + key + ": must be true or false", ERROR, entry->line, entry->order);
			Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section][key], WARNING, 0, entry->order + 1);
			entry->value = Config.defaultValues[section][key];
		}

		if (key == "priority" && !Utils::valid_number(entry->value, 0, 999)) {
			Utils::error_add(entry->filename, "[" + section + "] " + key + ": must be a value between 0 and 999", ERROR, entry->line, entry->order);
			Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section][key], WARNING, 0, entry->order + 1);
			entry->value = Config.defaultValues[section][key];
		}
		if (key == "startsecs" && !Utils::valid_number(entry->value, 0, 3600)) {
			Utils::error_add(entry->filename, "[" + section + "] " + key + ": must be a value between 0 and 3600", ERROR, entry->line, entry->order);
			Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section][key], WARNING, 0, entry->order + 1);
			entry->value = Config.defaultValues[section][key];
		}
		if (key == "startretries" && !Utils::valid_number(entry->value, 0, 100)) {
			Utils::error_add(entry->filename, "[" + section + "] " + key + ": must be a value between 0 and 100", ERROR, entry->line, entry->order);
			Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section][key], WARNING, 0, entry->order + 1);
			entry->value = Config.defaultValues[section][key];
		}
		if (key == "stopwaitsecs" && !Utils::valid_number(entry->value, 1, 3600)) {
			Utils::error_add(entry->filename, "[" + section + "] " + key + ": mmust be a value between 1 and 3600", ERROR, entry->line, entry->order);
			Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section][key], WARNING, 0, entry->order + 1);
			entry->value = Config.defaultValues[section][key];
		}

		if (key == "autorestart" && !Utils::valid_autorestart(entry->value)) {
			Utils::error_add(entry->filename, "[" + section + "] " + key + ": must be false, unexpected or true", ERROR, entry->line, entry->order);
			Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section][key], WARNING, 0, entry->order + 1);
			entry->value = Config.defaultValues[section][key];
		}

		if (key == "exitcodes" && !Utils::valid_code(entry->value)) {
			Utils::error_add(entry->filename, "[" + section + "] " + key + ": must be comma-separated numbers between 0 and 255", ERROR, entry->line, entry->order);
			Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section][key], WARNING, 0, entry->order + 1);
			entry->value = Config.defaultValues[section][key];
		}

		if (key == "stopsignal" && !Utils::valid_signal(entry->value)) {
			Utils::error_add(entry->filename, "[" + section + "] " + key + ": must be a valid signal (HUP, INT, QUIT, KILL, TERM, USR1, USR2)", ERROR, entry->line, entry->order);
			Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section][key], WARNING, 0, entry->order + 1);
			entry->value = Config.defaultValues[section][key];
		}

		if (key == "umask" && entry->value != "inherit") {
			if (!Utils::valid_umask(entry->value)) {
				Utils::error_add(entry->filename, "[" + section + "] " + key + ": must be in octal format", ERROR, entry->line, entry->order);
				Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section][key], WARNING, 0, entry->order + 1);
				entry->value = Config.defaultValues[section][key];
			}
		}

		if (key == "user") {
			if (!Utils::valid_user(entry->value)) {
				Utils::error_add(entry->filename, "[" + section + "] " + key + ": invalid user", CRITICAL, entry->line, entry->order);
				entry->value = "";
			} else {
				if (!Config.is_root && Utils::toLower(entry->value) != "do not switch") {
					struct passwd* pw = getpwuid(getuid());
					if (pw && pw->pw_name != entry->value && entry->value != std::to_string(getuid()))
						Utils::error_add(entry->filename, "[" + section + "] " + key + ": cannot switch user, not running as root", CRITICAL, entry->line, entry->order);
				}
			}
		}

		if (key == "stdout_logfile") {
			if (Utils::toUpper(entry->value) == "AUTO") {
				std::string childlogdir = TaskMaster.childlogdir;
				entry->value = section.substr(8) + "_stdout.log";
				if (!Utils::valid_path(entry->value, childlogdir)) {
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": invalid path - " + std::string(strerror(errno)), ERROR, entry->line, entry->order);
					entry->value = "NONE";
				}
			} else if (Utils::toUpper(entry->value) != "NONE") {
				if (!Utils::valid_path(entry->value, dir)) {
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": invalid path - " + std::string(strerror(errno)), ERROR, entry->line, entry->order);
					if (entry->value != Config.defaultValues[section][key]) {
						Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section][key], WARNING, 0, entry->order + 1);
						entry->value = Config.defaultValues[section][key];
						if (!Utils::valid_path(entry->value, dir)) {
							Utils::error_add(entry->filename, "[" + section + "] " + key + ": failed to use default value - " + std::string(strerror(errno)), ERROR, entry->line, entry->order);
							entry->value = "NONE";
						}
					} else entry->value = "NONE";
				}
			}
			if (Utils::toUpper(entry->value) != "NONE") entry->value = Utils::expand_path(entry->value, dir);
		}
		if (key == "stdout_logfile_maxbytes") {
			long bytes = Utils::parse_size(entry->value);
			if (bytes == -1 || !Utils::valid_number(std::to_string(bytes), 0, 1024 * 1024 * 1024)) {
				Utils::error_add(entry->filename, "[" + section + "] " + key + ": must be a value between 0 bytes and 1024 MB", ERROR, entry->line, entry->order);
				Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section][key], WARNING, 0, entry->order + 1);
				entry->value = Config.defaultValues[section][key];
			}
		}
		if (key == "stdout_logfile_backups" && !Utils::valid_number(entry->value, 0, 1000)) {
			Utils::error_add(entry->filename, "[" + section + "] " + key + ": must be a value between 0 and 1000", ERROR, entry->line, entry->order);
			Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section][key], WARNING, 0, entry->order + 1);
			entry->value = Config.defaultValues[section][key];
		}

		if (key == "stderr_logfile") {
			if (Utils::toUpper(entry->value) == "AUTO") {
				std::string childlogdir = TaskMaster.childlogdir;
				entry->value = section.substr(8) + "_stderr.log";
				if (!Utils::valid_path(entry->value, childlogdir)) {
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": invalid path - " + std::string(strerror(errno)), ERROR, entry->line, entry->order);
					entry->value = "NONE";
				}
			} else if (Utils::toUpper(entry->value) != "NONE") {
				if (!Utils::valid_path(entry->value, dir)) {
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": invalid path - " + std::string(strerror(errno)), ERROR, entry->line, entry->order);
					if (entry->value != Config.defaultValues[section][key]) {
						Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section][key], WARNING, 0, entry->order + 1);
						entry->value = Config.defaultValues[section][key];
						if (!Utils::valid_path(entry->value, dir)) {
							Utils::error_add(entry->filename, "[" + section + "] " + key + ": failed to use default value - " + std::string(strerror(errno)), ERROR, entry->line, entry->order);
							entry->value = "NONE";
						}
					} else entry->value = "NONE";
				}
			}
			if (Utils::toUpper(entry->value) != "NONE") entry->value = Utils::expand_path(entry->value, dir);
		}
		if (key == "stderr_logfile_maxbytes") {
			long bytes = Utils::parse_size(entry->value);
			if (bytes == -1 || !Utils::valid_number(std::to_string(bytes), 0, 1024 * 1024 * 1024)) {
				Utils::error_add(entry->filename, "[" + section + "] " + key + ": must be a value between 0 bytes and 1024 MB", ERROR, entry->line, entry->order);
				Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section][key], WARNING, 0, entry->order + 1);
				entry->value = Config.defaultValues[section][key];
			}
		}
		if (key == "stderr_logfile_backups" && !Utils::valid_number(entry->value, 0, 1000)) {
			Utils::error_add(entry->filename, "[" + section + "] " + key + ": must be a value between 0 and 1000", ERROR, entry->line, entry->order);
			Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section][key], WARNING, 0, entry->order + 1);
			entry->value = Config.defaultValues[section][key];
		}

		if (key == "serverurl") {
			if (Utils::toUpper(entry->value) != "AUTO" && !Utils::valid_serverurl(entry->value)) {
				Utils::error_add(entry->filename, "[" + section + "] " + key + ": invalid format", ERROR, entry->line, entry->order);
				Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section][key], WARNING, 0, entry->order + 1);
				entry->value = Config.defaultValues[section][key];
			}
			if (Utils::toUpper(entry->value) == "AUTO") {
				std::string url;
				if (!TaskMaster.unix_server.disabled)					url = "unix://" + TaskMaster.unix_server.file;
				if (url.empty() && !TaskMaster.inet_server.disabled)	url = "http://" + TaskMaster.inet_server.url;
				entry->value = url;
			}
		}

		return (entry->value);
	}

#pragma endregion

#pragma region "Expand Vars (cambiar)"

	std::string Program::expand_vars(std::map<std::string, std::string>& env, const std::string& key) {
		if (key.empty()) return {};

		ConfigParser::ConfigEntry *entry = Config.get_value_entry(section, key);
		if (!entry) return {};

		std::string original_value = entry->value;

		try {
			entry->value = Utils::environment_expand(env, entry->value);
			if (key == "environment") {
				if (!Utils::environment_validate(entry->value)) {
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": invalid variable format", ERROR, entry->line, entry->order);
					entry->value = "";
				} else Utils::environment_add_batch(env, entry->value);
			}
			else entry->value = Utils::remove_quotes(entry->value);
		}
		catch(const std::exception& e) {
			Utils::error_add(entry->filename, "[" + section + "] " + key + ": unclosed quote or unfinished escape sequence", ERROR, entry->line, entry->order);
			if (key != "environment") {
				Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section][key], WARNING, 0, entry->order + 1);
				entry->value = Config.defaultValues[section][key];
			} else entry->value = "";
		}

		std::string final_value = validate(key, entry);
		entry->value = original_value;
		return (final_value);
	}

#pragma endregion

#pragma region "Initialize"

	void Program::initialize() {
		std::string configFile;
		uint16_t	order = 0;
		disabled = false;

		ConfigParser::ConfigEntry *entry = Config.get_value_entry(section, "directory");
		if (!entry)	  configFile = Utils::expand_path(".", "", true, false);
		else		{ configFile = entry->filename; order = entry->order + 1; }

		std::map<std::string, std::string> env;
		Utils::environment_clone(env, TaskMaster.environment);

		std::string HERE			= Utils::environment_get(env, "HERE");
		std::string HOST_NAME		= Utils::environment_get(env, "HOST_NAME");
		std::string GROUP_NAME		= Utils::environment_get(env, "GROUP_NAME");
		std::string GROUP_NAMES		= Utils::environment_get(env, "GROUP_NAMES");
		std::string PROGRAM_NAME	= Utils::environment_get(env, "PROGRAM_NAME");
		std::string NUMPROCS		= Utils::environment_get(env, "NUMPROCS");
		std::string PROCESS_NUM		= Utils::environment_get(env, "PROCESS_NUM");

		char hostname[255];
		Utils::environment_add(env, "HOST_NAME", (!gethostname(hostname, sizeof(hostname))) ? std::string(hostname) : "unknown");
		if (!configFile.empty()) Utils::environment_add(env, "HERE", std::filesystem::path(configFile).parent_path());

		std::string	g_programs, group_name, group_names;
		uint16_t	g_order = 65535;

		for (auto& [group, keys] : Config.sections) {
			if (group.substr(0, 6) == "group:") {
				ConfigParser::ConfigEntry *g_entry = Config.get_value_entry(group, "programs");
				if (!g_entry || g_entry->value.empty()) continue;

				try {
					std::map<std::string, std::string> g_env;
					Utils::environment_initialize(g_env);
					g_programs = Utils::environment_expand(g_env, g_entry->value);
					g_programs = Utils::remove_quotes(g_programs);
				}
				catch(const std::exception& e) { g_programs = entry->value; }

				if (group.substr(6) == name) Utils::error_add(configFile, "[" + section + "] programs: Program '" + name + "' has same name as group '" + group.substr(0, 6) + "'. Program will take precedence in ambiguous commands", WARNING, 0, order);

				std::set<std::string> program_names;
				std::stringstream ss(g_programs); std::string token;
				while (std::getline(ss, token, ',')) program_names.insert(Utils::trim(token));

				if (program_names.find(name) != program_names.end()) {
					if (g_entry->order < g_order) {
						g_order = g_entry->order;
						group_name = group.substr(6);
					}
					if (group_names.empty())	group_names = group.substr(6);
					else						group_names = group_names + ", " + group.substr(6);
				}
			}
		}

		Utils::environment_del(env, "TASKMASTER_PROCESS_NAME");
		Utils::environment_del(env, "TASKMASTER_SERVER_URL");
		Utils::environment_add(env, "TASKMASTER_ENABLED", "1");
		if (!group_name.empty()) {
			Utils::environment_add(env, "TASKMASTER_GROUP_NAME", group_name);
			Utils::environment_add(env, "GROUP_NAME", group_name);
		}
		if (!group_names.empty()) {
			Utils::environment_add(env, "TASKMASTER_GROUP_NAMES", group_names);
			Utils::environment_add(env, "GROUP_NAMES", group_name);
		}

		numprocs		= Utils::parse_number(expand_vars(env, "numprocs"), 0, 10000, 1);
		numprocs_start	= Utils::parse_number(expand_vars(env, "numprocs_start"), 0, 65535, 0);

		Utils::environment_add(env, "NUMPROCS", std::to_string(numprocs));

		uint16_t current_process = 0;
		uint16_t current_process_num = numprocs_start;
		process.reserve(numprocs);

		while (current_process < numprocs && !disabled) {
			try {
				process.emplace_back();
				Process& proc = process.back();
	
				proc.process_num = current_process;
				proc.program_name = name;

				Utils::environment_add(proc.environment, env);
				Utils::environment_add(proc.environment, "PROGRAM_NAME", name);
				Utils::environment_add(proc.environment, "PROCESS_NUM", std::to_string(current_process_num++));

				proc.directory	= expand_vars(proc.environment, "directory");
				proc.name		= expand_vars(proc.environment, "process_name");

				// Expandir variables
				// Dividir en vector
				// Por cada item hacer globbing
				// Sustituir item por vector devuelto por globbing
				// Command = vector[0]
				// Args es el vector directamente

				entry = Config.get_value_entry(section, "command");
				if (entry) {
					try { entry->value = Utils::environment_expand(proc.environment, entry->value); }
					catch (const std::exception& e) { Utils::error_add(entry->filename, "[" + section + "] command: unclosed quote or unfinished escape sequence", ERROR, entry->line, entry->order); }

					proc.arguments = Utils::globbing_expand(Utils::parse_arguments(entry->value));

					if (proc.arguments.empty() || proc.arguments[0].empty()) {
						Utils::error_add(entry->filename, "[" + section + "] command: empty value", ERROR, entry->line, entry->order);
						disabled = true;
					} else if ((proc.command = Utils::parse_executable(proc.arguments[0])).empty()) {
						Utils::error_add(entry->filename, "[" + section + "] command: must be a valid executable", ERROR, entry->line, entry->order);
						disabled = true;
					}
				} else {
					Utils::error_add(configFile, "[" + section + "] command: required", ERROR, 0, order);
					disabled = true;
				}

				proc.priority					= Utils::parse_number(expand_vars(proc.environment, "priority"), 0, 999, 999);
				proc.autostart					= Utils::parse_bool(expand_vars(proc.environment, "autostart"));
				proc.autorestart				= Utils::parse_bool(expand_vars(proc.environment, "autorestart"));
				proc.startsecs					= Utils::parse_number(expand_vars(proc.environment, "startsecs"), 0, 3600, 1);
				proc.startretries				= Utils::parse_number(expand_vars(proc.environment, "startretries"), 1, 100, 3);

				std::stringstream ss(expand_vars(proc.environment, "exitcodes")); std::string token;
				while (std::getline(ss, token, ',')) proc.exitcodes.push_back(static_cast<uint8_t>(std::stoi(token)));

				proc.stopsignal					= Utils::parse_signal(expand_vars(proc.environment, "stopsignal"));
				proc.stopwaitsecs				= Utils::parse_number(expand_vars(proc.environment, "stopwaitsecs"), 1, 3600, 10);
				proc.stopasgroup				= Utils::parse_bool(expand_vars(proc.environment, "stopasgroup"));
				proc.killasgroup				= Utils::parse_bool(expand_vars(proc.environment, "killasgroup"));
				proc.tty_mode					= Utils::parse_bool(expand_vars(proc.environment, "tty_mode"));

				std::string umask				= expand_vars(proc.environment, "umask");
				if (umask.empty() || umask == "inherit")	proc.umask = TaskMaster.umask;
				else										proc.umask = static_cast<uint16_t>(std::stoi(umask, nullptr, 8));

				proc.user						= expand_vars(proc.environment, "user");
				proc.redirect_stderr			= Utils::parse_bool(expand_vars(proc.environment, "redirect_stderr"));
				proc.stdout_logfile				= expand_vars(proc.environment, "stdout_logfile");
				proc.stdout_logfile_maxbytes	= Utils::parse_size(expand_vars(proc.environment, "stdout_logfile_maxbytes"));
				proc.stdout_logfile_backups		= Utils::parse_number(expand_vars(proc.environment, "stdout_logfile_backups"), 0, 1000, 10);
				proc.stdout_logfile_syslog		= Utils::parse_bool(expand_vars(proc.environment, "stdout_logfile_syslog"));
				proc.stderr_logfile				= expand_vars(proc.environment, "stderr_logfile");
				proc.stderr_logfile_maxbytes	= Utils::parse_size(expand_vars(proc.environment, "stderr_logfile_maxbytes"));
				proc.stderr_logfile_backups		= Utils::parse_number(expand_vars(proc.environment, "stderr_logfile_backups"), 0, 1000, 10);
				proc.stderr_logfile_syslog		= Utils::parse_bool(expand_vars(proc.environment, "stderr_logfile_syslog"));
				proc.serverurl					= expand_vars(proc.environment, "serverurl");

				Utils::environment_add(proc.environment, "TASKMASTER_PROCESS_NAME", proc.name);
				if (!proc.serverurl.empty())	Utils::environment_add(proc.environment, "TASKMASTER_SERVER_URL", proc.serverurl);
				else							Utils::environment_del(proc.environment, "TASKMASTER_SERVER_URL");

				if (HERE.empty())				Utils::environment_del(proc.environment, "HERE");
				else							Utils::environment_add(proc.environment, "HERE", HERE);
				if (HOST_NAME.empty())			Utils::environment_del(proc.environment, "HOST_NAME");
				else							Utils::environment_add(proc.environment, "HOST_NAME", HOST_NAME);
				if (GROUP_NAME.empty())			Utils::environment_del(proc.environment, "GROUP_NAME");
				else							Utils::environment_add(proc.environment, "GROUP_NAME", GROUP_NAME);
				if (GROUP_NAMES.empty())		Utils::environment_del(proc.environment, "GROUP_NAMES");
				else							Utils::environment_add(proc.environment, "GROUP_NAMES", GROUP_NAMES);
				if (NUMPROCS.empty())			Utils::environment_del(proc.environment, "NUMPROCS");
				else							Utils::environment_add(proc.environment, "NUMPROCS", NUMPROCS);
				if (PROCESS_NUM.empty())		Utils::environment_del(proc.environment, "PROCESS_NUM");
				else							Utils::environment_add(proc.environment, "PROCESS_NUM", PROCESS_NUM);

				expand_vars(proc.environment, "environment");

				current_process++;
			} catch(const std::exception& e) {
				disabled = true;
				return;
			}
		}
	}

#pragma endregion
