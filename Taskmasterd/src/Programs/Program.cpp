/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Program.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/02 17:23:05 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/11 20:13:14 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Taskmaster/Taskmaster.hpp"

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

#pragma region "Configuration"

	#pragma region "Validate Helpers"

		#pragma region "Directory"

			std::string Program::validate_directory(const std::string& key, ConfigParser::ConfigEntry *entry) {
				if (key.empty() || !entry) return {};

				std::string default_dir = tskm.directory;
				if (entry->value != "do not change") {
					if (!Utils::valid_path(entry->value, "", true)) {
						Utils::error_add(entry->filename, "[" + section + "] " + key + ": invalid path - " + std::string(strerror(errno)), ERROR, entry->line, entry->order + 2);
						if (!Utils::valid_path(default_dir, "", true)) {
							Utils::error_add(entry->filename, "[" + section + "] " + key + ": failed to use default value - " + std::string(strerror(errno)), ERROR,  entry->line, entry->order + 3);
							entry->value = ""; disabled = true;
						}else {
							entry->value = Utils::expand_path(default_dir, "", true, false);
							Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + ((default_dir.empty()) ? "current directory" : default_dir), WARNING,  entry->line, entry->order + 3);
						}
					} else entry->value = Utils::expand_path(entry->value, "", true, false);
				} else {
					if (!Utils::valid_path(default_dir, "", true)) {
						Utils::error_add(entry->filename, "[" + section + "] " + key + ": failed to use default value - " + std::string(strerror(errno)), ERROR,  entry->line, entry->order + 2);
						entry->value = ""; disabled = true;
					}
					else entry->value = Utils::expand_path(default_dir, "", true, false);
				}

				return (entry->value);
			}

		#pragma endregion

		#pragma region "Boolean"

			std::string Program::validate_boolean(const std::string& key, ConfigParser::ConfigEntry *entry) {
				if (key.empty() || !entry) return {};

				if (!Utils::valid_boolean(entry->value)) {
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": must be true or false", ERROR, entry->line, entry->order + 2);
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section.substr(0, 8)][key], WARNING, entry->line, entry->order + 3);
					entry->value = Config.defaultValues[section.substr(0, 8)][key];
				}

				return (entry->value);
			}

		#pragma endregion

		#pragma region "Number"

			std::string Program::validate_number(const std::string& key, ConfigParser::ConfigEntry *entry, long min, long max) {
				if (key.empty() || !entry) return {};

				if (!Utils::valid_number(entry->value, min, max)) {
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": must be a value between " + std::to_string(min) + " and " + std::to_string(max), ERROR, entry->line, entry->order + 2);
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section.substr(0, 8)][key], WARNING, entry->line, entry->order + 3);
					entry->value = Config.defaultValues[section.substr(0, 8)][key];
				}

				return (entry->value);
			}

		#pragma endregion

		#pragma region "Auto Restart"

			std::string Program::validate_autorestart(const std::string& key, ConfigParser::ConfigEntry *entry) {
				if (key.empty() || !entry) return {};

				if (!Utils::valid_autorestart(entry->value)) {
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": must be true, unexpected or false", ERROR, entry->line, entry->order + 2);
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section.substr(0, 8)][key], WARNING, entry->line, entry->order + 3);
					entry->value = Config.defaultValues[section.substr(0, 8)][key];
				}

				return (entry->value);
			}

		#pragma endregion

		#pragma region "Exit Codes"

			std::string Program::validate_exitcodes(const std::string& key, ConfigParser::ConfigEntry *entry) {
				if (key.empty() || !entry) return {};

				if (!Utils::valid_exitcodes(entry->value)) {
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": must be comma-separated numbers between 0 and 255", ERROR, entry->line, entry->order + 2);
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section.substr(0, 8)][key], WARNING, entry->line, entry->order + 3);
					entry->value = Config.defaultValues[section.substr(0, 8)][key];
				}

				return (entry->value);
			}

		#pragma endregion

		#pragma region "Stop Signal"

			std::string Program::validate_stopsignal(const std::string& key, ConfigParser::ConfigEntry *entry) {
				if (key.empty() || !entry) return {};

				if (!Utils::valid_signal(entry->value)) {
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": must be a valid signal (HUP, INT, QUIT, KILL, TERM, USR1, USR2)", ERROR, entry->line, entry->order + 2);
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section.substr(0, 8)][key], WARNING, entry->line, entry->order + 3);
					entry->value = Config.defaultValues[section.substr(0, 8)][key];
				}

				return (entry->value);
			}

		#pragma endregion
		
		#pragma region "Umask"

			std::string Program::validate_umask(const std::string& key, ConfigParser::ConfigEntry *entry) {
				if (key.empty() || !entry) return {};

				if (entry->value != "inherit") {
					if (!Utils::valid_umask(entry->value)) {
						Utils::error_add(entry->filename, "[" + section + "] " + key + ": must be in octal format", ERROR, entry->line, entry->order + 2);
						Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section.substr(0, 8)][key], WARNING, entry->line, entry->order + 3);
						entry->value = Config.defaultValues[section.substr(0, 8)][key];
					}
				}

				return (entry->value);
			}

		#pragma endregion
		
		#pragma region "User"

			std::string Program::validate_user(const std::string& key, ConfigParser::ConfigEntry *entry) {
				if (key.empty() || !entry) return {};

				if (!Utils::valid_user(entry->value)) {
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": invalid user", ERROR, entry->line, entry->order + 2);
					entry->value = ""; entry->value = ""; disabled = true;
				} else {
					if (!Config.is_root && Utils::toLower(entry->value) != "do not switch") {
						struct passwd* pw = getpwuid(getuid());
						if (pw && pw->pw_name != entry->value && entry->value != std::to_string(getuid())) {
							Utils::error_add(entry->filename, "[" + section + "] " + key + ": cannot switch user, not running as root", ERROR, entry->line, entry->order + 2);
							entry->value = ""; disabled = true;
						}
					}
					else if (!Config.is_root && Utils::toLower(entry->value) == "do not switch" && tskm.user != "do not switch") {
						struct passwd* pw = getpwuid(getuid());
						if (pw && pw->pw_name != tskm.user && tskm.user != std::to_string(getuid())) {
							Utils::error_add(entry->filename, "[" + section + "] " + key + ": cannot switch user, not running as root", ERROR, entry->line, entry->order + 2);
							entry->value = ""; disabled = true;
						}
					}

					if (Config.is_root && Utils::toLower(entry->value) == "do not switch" && tskm.user == "do not switch") tskm.root_warning = true;
				}

				return (entry->value);
			}

		#pragma endregion

		#pragma region "Logfile"

			std::string Program::validate_logfile(const std::string& key, ConfigParser::ConfigEntry *entry, const std::string& dir) {
				if (key.empty() || !entry) return {};

				std::string final_dir = dir;

				if (Utils::toUpper(entry->value) == "AUTO") {
					final_dir = tskm.childlogdir;
					entry->value = section.substr(8) + "_" + key.substr(0, 6) + ".log";
					if (!Utils::valid_path(entry->value, final_dir)) {
						Utils::error_add(entry->filename, "[" + section + "] " + key + ": failed to use default value - " + std::string(strerror(errno)), ERROR, entry->line, entry->order + 2);
						entry->value = "NONE";
					}
				} else if (Utils::toUpper(entry->value) != "NONE") {
					if (!Utils::valid_path(entry->value, dir)) {
						Utils::error_add(entry->filename, "[" + section + "] " + key + ": invalid path - " + std::string(strerror(errno)), ERROR, entry->line, entry->order + 2);
						entry->value = "NONE";
					}
				}
				if (Utils::toUpper(entry->value) != "NONE") entry->value = Utils::expand_path(entry->value, final_dir);
				if (entry->value.empty()) entry->value = "NONE";

				return (entry->value);
			}

		#pragma endregion

		#pragma region "Size"

			std::string Program::validate_size(const std::string& key, ConfigParser::ConfigEntry *entry) {
				if (key.empty() || !entry) return {};

				long bytes = Utils::parse_size(entry->value);
				if (bytes == -1 || !Utils::valid_number(std::to_string(bytes), 0, 1024 * 1024 * 1024)) {
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": must be a value between 0 bytes and 1024 MB", ERROR, entry->line, entry->order + 2);
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section.substr(0, 8)][key], WARNING, entry->line, entry->order + 3);
					entry->value = Config.defaultValues[section.substr(0, 8)][key];
				}

				return (entry->value);
			}

		#pragma endregion

		#pragma region "Server URL"

			std::string Program::validate_serverurl(const std::string& key, ConfigParser::ConfigEntry *entry) {
				if (key.empty() || !entry) return {};

				if (Utils::toUpper(entry->value) != "AUTO" && !Utils::valid_serverurl(entry->value)) {
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": invalid format", ERROR, entry->line, entry->order);
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section.substr(0, 8)][key], WARNING, 0, entry->order + 1);
					entry->value = Config.defaultValues[section.substr(0, 8)][key];
				}
				if (Utils::toUpper(entry->value) == "AUTO") {
					std::string url;
					if (!tskm.unix_server.disabled)					url = "unix://" + tskm.unix_server.file;
					if (url.empty() && !tskm.inet_server.disabled)	url = "http://" + tskm.inet_server.url;
					entry->value = url;
				}

				return (entry->value);
			}

		#pragma endregion

	#pragma endregion

	#pragma region "Validate"

		std::string Program::validate(const std::string& key, ConfigParser::ConfigEntry *entry) {
			if (key.empty() || !entry) return {};

			static std::string dir = "";

			if (key == "directory")					dir = validate_directory(key, entry);

			if (key == "tty_mode")					validate_boolean(key, entry);
			if (key == "autostart")					validate_boolean(key, entry);
			if (key == "stopasgroup")				validate_boolean(key, entry);
			if (key == "killasgroup")				validate_boolean(key, entry);
			if (key == "redirect_stderr")			validate_boolean(key, entry);

			if (key == "numprocs")					validate_number(key, entry, 1, 65535);
			if (key == "numprocs_start")			validate_number(key, entry, 0, 65535);
			if (key == "priority")					validate_number(key, entry, 0, 999);
			if (key == "startsecs")					validate_number(key, entry, 0, 3600);
			if (key == "startretries")				validate_number(key, entry, 0, 100);
			if (key == "stopwaitsecs")				validate_number(key, entry, 1, 3600);

			if (key == "autorestart")				validate_autorestart(key, entry);
			if (key == "exitcodes")					validate_exitcodes(key, entry);
			if (key == "stopsignal")				validate_stopsignal(key, entry);

			if (key == "umask")						validate_umask(key, entry);
			if (key == "user")						validate_user(key, entry);

			if (key == "stdout_logfile")			validate_logfile(key, entry, dir);
			if (key == "stdout_logfile_maxbytes")	validate_size(key, entry);
			if (key == "stdout_logfile_backups")	validate_number(key, entry, 0, 1000);
			if (key == "stdout_logfile_syslog")		validate_boolean(key, entry);

			if (key == "stderr_logfile")			validate_logfile(key, entry, dir);
			if (key == "stderr_logfile_maxbytes")	validate_size(key, entry);
			if (key == "stderr_logfile_backups")	validate_number(key, entry, 0, 1000);
			if (key == "stderr_logfile_syslog")		validate_boolean(key, entry);

			if (key == "serverurl")					validate_serverurl(key, entry);

			return (entry->value);
		}

	#pragma endregion

	#pragma region "Expand Vars"

		std::string Program::expand_vars(std::map<std::string, std::string>& env, const std::string& key) {
			if (key.empty()) return {};

			ConfigParser::ConfigEntry *entry = Config.get_value_entry(section, key);
			if (!entry) return {};

			std::string original_value = entry->value;
			bool restore_original = (entry->value.find("$PROCESS_NUM") != std::string::npos || entry->value.find("${PROCESS_NUM}") != std::string::npos || entry->value.find("${PROCESS_NUM:") != std::string::npos);

			try {
				entry->value = Utils::environment_expand(env, entry->value);
				if (key == "environment") {
					if (!Utils::environment_validate(entry->value)) {
						Utils::error_add(entry->filename, "[" + section + "] " + key + ": invalid variable format", ERROR, entry->line, entry->order + 1);
						entry->value = ""; original_value = "";
					} else Utils::environment_add_batch(env, entry->value);
				}
				else {
					entry->value = Utils::remove_quotes(entry->value);
					if (entry->value.empty()) {
						Utils::error_add(entry->filename, "[" + section + "] " + key + ": empty value", ERROR, entry->line, entry->order);
						if (key != "user") {
							Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section.substr(0, 8)][key], WARNING, entry->line, entry->order + 1);
							entry->value = Config.defaultValues[section.substr(0, 8)][key];
						} else { disabled = true; return {}; }
					}
				}
			}
			catch(const std::exception& e) {
				Utils::error_add(entry->filename, "[" + section + "] " + key + ": unclosed quote or unfinished escape sequence", ERROR, entry->line, entry->order);
				if (key != "environment" && key != "user") {
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section.substr(0, 8)][key], WARNING, entry->line, entry->order + 1);
					entry->value = Config.defaultValues[section.substr(0, 8)][key];
				}
				else if (key == "user") { disabled = true; return {}; }
				else { entry->value = ""; original_value = ""; }
			}

			std::string final_value = validate(key, entry);
			if (restore_original) entry->value = original_value;

			return (final_value);
		}

	#pragma endregion

	#pragma region "Add Groups"

		void Program::add_groups(std::map<std::string, std::string>& env, std::string& configFile, uint16_t order) {
			std::string	g_programs, group_name, group_names;
			uint16_t	g_order = 65535;

			for (auto& [group, keys] : Config.sections) {
				if (group.substr(0, 6) == "group:") {
					ConfigParser::ConfigEntry *g_entry = Config.get_value_entry(group, "programs");
					if (!g_entry || g_entry->value.empty()) continue;

					try {
						g_programs = Utils::environment_expand(env, g_entry->value);
						g_programs = Utils::remove_quotes(g_programs);
					}
					catch(const std::exception& e) { g_programs = g_entry->value; }

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
						groups.push_back(group.substr(6));
					}
				}
			}

			if (!group_name.empty()) {
				Utils::environment_add(env, "TASKMASTER_GROUP_NAME", group_name);
				Utils::environment_add(env, "GROUP_NAME", group_name);
			}
			if (!group_names.empty()) {
				Utils::environment_add(env, "TASKMASTER_GROUP_NAMES", group_names);
				Utils::environment_add(env, "GROUP_NAMES", group_name);
			}
		}

	#pragma endregion

	#pragma region "Initialize"

		void Program::initialize() {
			std::string configFile;
			uint16_t	order = 0;
			disabled = needs_restart = false;

			ConfigParser::ConfigEntry *entry = Config.get_value_entry(section, "directory");
			if (!entry)	  configFile = Utils::expand_path(".", "", true, false);
			else		{ configFile = entry->filename; order = entry->order; }

			std::map<std::string, std::string> env;
			Utils::environment_clone(env, tskm.environment);

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
			add_groups(env, configFile, order);

			numprocs		= Utils::parse_number(expand_vars(env, "numprocs"), 0, 10000, 1);
			numprocs_start	= Utils::parse_number(expand_vars(env, "numprocs_start"), 0, 65535, 0);

			Utils::environment_add(env, "NUMPROCS", std::to_string(numprocs));
			Utils::environment_del(env, "TASKMASTER_PROCESS_NAME");
			Utils::environment_del(env, "TASKMASTER_SERVER_URL");
			Utils::environment_add(env, "TASKMASTER_ENABLED", "1");

			uint16_t current_process = 0;
			uint16_t current_process_num = numprocs_start;
			process.reserve(numprocs);

			while (current_process < numprocs && !disabled) {
				try {
					process.emplace_back();
					Process& proc = process.back();

					proc.program_name = name;
					proc.process_num = current_process;

					Utils::environment_add(proc.environment, env);
					Utils::environment_add(proc.environment, "PROGRAM_NAME", name);
					Utils::environment_add(proc.environment, "PROCESS_NUM", std::to_string(current_process_num++));

					proc.directory	= expand_vars(proc.environment, "directory");
					proc.name		= expand_vars(proc.environment, "process_name");

					entry = Config.get_value_entry(section, "command");
					if (entry) {
						try { entry->value = Utils::environment_expand(proc.environment, entry->value); }
						catch (const std::exception& e) { Utils::error_add(entry->filename, "[" + section + "] command: unclosed quote or unfinished escape sequence", ERROR, entry->line, entry->order); }

						proc.arguments = Utils::globbing_expand(Utils::parse_arguments(entry->value));

						if (proc.arguments.empty() || proc.arguments[0].empty()) {
							Utils::error_add(entry->filename, "[" + section + "] command: empty value", ERROR, entry->line, entry->order);
							Utils::error_add(entry->filename, "[" + section + "] command: required", ERROR, 0, entry->order + 1);
							disabled = true;
						} else if ((proc.command = Utils::parse_executable(proc.arguments[0])).empty()) {
							Utils::error_add(entry->filename, "[" + section + "] command: must be a valid executable", ERROR, entry->line, entry->order);
							disabled = true;
						}
					} else {
						Utils::error_add(configFile, "[" + section + "] command: required", ERROR, 0, order);
						disabled = true;
					}

					proc.priority								= Utils::parse_number(expand_vars(proc.environment, "priority"), 0, 999, 999);
					proc.autostart								= Utils::parse_boolean(expand_vars(proc.environment, "autostart"));
					proc.autorestart							= Utils::parse_boolean(expand_vars(proc.environment, "autorestart"));
					proc.startsecs								= Utils::parse_number(expand_vars(proc.environment, "startsecs"), 0, 3600, 1);
					proc.startretries							= Utils::parse_number(expand_vars(proc.environment, "startretries"), 1, 100, 3);
					proc.stopsignal								= Utils::parse_signal(expand_vars(proc.environment, "stopsignal"));
					proc.stopwaitsecs							= Utils::parse_number(expand_vars(proc.environment, "stopwaitsecs"), 1, 3600, 10);
					proc.stopasgroup							= Utils::parse_boolean(expand_vars(proc.environment, "stopasgroup"));
					proc.killasgroup							= Utils::parse_boolean(expand_vars(proc.environment, "killasgroup"));
					proc.tty_mode								= Utils::parse_boolean(expand_vars(proc.environment, "tty_mode"));
					proc.user									= expand_vars(proc.environment, "user");
					proc.redirect_stderr						= Utils::parse_boolean(expand_vars(proc.environment, "redirect_stderr"));
					proc.stdout_logfile							= expand_vars(proc.environment, "stdout_logfile");
					proc.stdout_logfile_maxbytes				= Utils::parse_size(expand_vars(proc.environment, "stdout_logfile_maxbytes"));
					proc.stdout_logfile_backups					= Utils::parse_number(expand_vars(proc.environment, "stdout_logfile_backups"), 0, 1000, 10);
					proc.stdout_logfile_syslog					= Utils::parse_boolean(expand_vars(proc.environment, "stdout_logfile_syslog"));
					proc.stderr_logfile							= expand_vars(proc.environment, "stderr_logfile");
					proc.stderr_logfile_maxbytes				= Utils::parse_size(expand_vars(proc.environment, "stderr_logfile_maxbytes"));
					proc.stderr_logfile_backups					= Utils::parse_number(expand_vars(proc.environment, "stderr_logfile_backups"), 0, 1000, 10);
					proc.stderr_logfile_syslog					= Utils::parse_boolean(expand_vars(proc.environment, "stderr_logfile_syslog"));
					proc.serverurl								= expand_vars(proc.environment, "serverurl");
					std::string umask							= expand_vars(proc.environment, "umask");
					if (umask.empty() || umask == "inherit")	proc.umask = tskm.umask;
					else										proc.umask = static_cast<uint16_t>(std::stoi(umask, nullptr, 8));

					std::stringstream ss(expand_vars(proc.environment, "exitcodes")); std::string token;
					while (std::getline(ss, token, ',')) proc.exitcodes.push_back(static_cast<uint8_t>(std::stoi(token)));

					Utils::environment_add(proc.environment, "TASKMASTER_PROCESS_NAME", proc.name);
					if (!proc.serverurl.empty())				Utils::environment_add(proc.environment, "TASKMASTER_SERVER_URL", proc.serverurl);
					else										Utils::environment_del(proc.environment, "TASKMASTER_SERVER_URL");
					if (HERE.empty())							Utils::environment_del(proc.environment, "HERE");
					else										Utils::environment_add(proc.environment, "HERE", HERE);
					if (HOST_NAME.empty())						Utils::environment_del(proc.environment, "HOST_NAME");
					else										Utils::environment_add(proc.environment, "HOST_NAME", HOST_NAME);
					if (GROUP_NAME.empty())						Utils::environment_del(proc.environment, "GROUP_NAME");
					else										Utils::environment_add(proc.environment, "GROUP_NAME", GROUP_NAME);
					if (GROUP_NAMES.empty())					Utils::environment_del(proc.environment, "GROUP_NAMES");
					else										Utils::environment_add(proc.environment, "GROUP_NAMES", GROUP_NAMES);
					if (NUMPROCS.empty())						Utils::environment_del(proc.environment, "NUMPROCS");
					else										Utils::environment_add(proc.environment, "NUMPROCS", NUMPROCS);
					if (PROCESS_NUM.empty())					Utils::environment_del(proc.environment, "PROCESS_NUM");
					else										Utils::environment_add(proc.environment, "PROCESS_NUM", PROCESS_NUM);

					expand_vars(proc.environment, "environment");

					current_process++;
					if (disabled) Utils::error_add(entry->filename, "[" + section + "] program '" + name + "' is disabled due to configuration errors", ERROR, entry->line, entry->order);
				} catch(const std::exception& e) {
					Utils::error_add(entry->filename, "[" + section + "] program '" + name + "' is disabled due to configuration errors", ERROR, entry->line, entry->order);
					disabled = true;
				}
			}
		}

	#pragma endregion

#pragma endregion
