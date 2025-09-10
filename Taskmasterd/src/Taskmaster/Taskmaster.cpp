/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Taskmaster.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/02 17:23:05 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/10 18:50:40 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Taskmaster/Taskmaster.hpp"

	#include <cstring>															// strerror()
	#include <sstream>															// std::ostringstream
	#include <iomanip>															// std::setw(), std::setfill()
	#include <filesystem>														// std::filesistem::path()
	#include <pwd.h>															// struct passwd, getpwuid()
	#include <unistd.h>															// gethostname(), fork(), setsid(), chdir(), close(), getpid()
	#include <sys/stat.h>														// umask()

#pragma endregion

#pragma region "Variables"

	Taskmaster tskm;

#pragma endregion

#pragma region "Constructors"

	Taskmaster::Taskmaster() : section("taskmasterd"), running(true), pid(0) {}

#pragma endregion

#pragma region "Configuration"

	#pragma region "Validate Helpers"

		#pragma region "Directory"

			std::string Taskmaster::validate_directory(const std::string& key, ConfigParser::ConfigEntry *entry) {
				if (key.empty() || !entry) return {};

				std::string default_dir = Utils::expand_path(".", "", true, false);
				if (!entry->value.empty() && entry->value != "do not change") {
					entry->value = Utils::expand_path(entry->value, "", true, false);
					if (!Utils::valid_path(entry->value, "", true)) {
						Utils::error_add(entry->filename, "[" + section + "] " + key + ": invalid path - " + std::string(strerror(errno)), ERROR, entry->line, entry->order + 2);
						if (!Utils::valid_path(default_dir, "", true)) {
							Utils::error_add(entry->filename, "[" + section + "] " + key + ": failed to use default value - " + std::string(strerror(errno)), CRITICAL, entry->line, entry->order + 3);
							entry->value = "";
						}else {
							entry->value = default_dir;
							Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + ((entry->value.empty()) ? "current directory" : entry->value), WARNING, 0, entry->order + 4);
						}
					}
				} else {
					if (entry->value.empty()) {
						Utils::error_add(entry->filename, "[" + section + "] " + key + ": empty value", ERROR, entry->line, entry->order + 2);
						Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + ((entry->value.empty()) ? "current directory" : entry->value), WARNING, 0, entry->order + 3);
						entry->value = default_dir;
					}
					if (!Utils::valid_path(default_dir, "", true)) {
						Utils::error_add(entry->filename, "[" + section + "] " + key + ": failed to use default value - " + std::string(strerror(errno)), CRITICAL, entry->line, entry->order + 4);
						entry->value = "";
					}
					else entry->value = default_dir;
				}

				return (entry->value);
			}

		#pragma endregion

		#pragma region "Boolean"

			std::string Taskmaster::validate_boolean(const std::string& key, ConfigParser::ConfigEntry *entry) {
				if (key.empty() || !entry) return {};

				if (!Utils::valid_boolean(entry->value)) {
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": must be true or false", ERROR, entry->line, entry->order + 2);
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section][key], WARNING, entry->line, entry->order + 3);
					entry->value = Config.defaultValues[section][key];
				}

				return (entry->value);
			}

		#pragma endregion

		#pragma region "Number"

			std::string Taskmaster::validate_number(const std::string& key, ConfigParser::ConfigEntry *entry, long min, long max) {
				if (key.empty() || !entry) return {};

				if (!Utils::valid_number(entry->value, min, max)) {
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": must be a value between " + std::to_string(min) + " and " + std::to_string(max), ERROR, entry->line, entry->order + 2);
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section][key], WARNING, entry->line, entry->order + 3);
					entry->value = Config.defaultValues[section][key];
				}

				return (entry->value);
			}

		#pragma endregion

		#pragma region "Umask"

			std::string Taskmaster::validate_umask(const std::string& key, ConfigParser::ConfigEntry *entry) {
				if (key.empty() || !entry) return {};

				if (!Utils::valid_umask(entry->value)) {
				Utils::error_add(entry->filename, "[" + section + "] " + key + ": must be in octal format", ERROR, entry->line, entry->order + 2);
				Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section][key], WARNING, entry->line, entry->order + 3);
				entry->value = Config.defaultValues[section][key];
			}

				return (entry->value);
			}

		#pragma endregion
		
		#pragma region "User"

			std::string Taskmaster::validate_user(const std::string& key, ConfigParser::ConfigEntry *entry) {
				if (key.empty() || !entry) return {};

				if (!Utils::valid_user(entry->value)) {
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": invalid user", CRITICAL, entry->line, entry->order + 2);
					entry->value = "";
				} else {
					if (!Config.is_root && Utils::toLower(entry->value) != "do not switch") {
						struct passwd* pw = getpwuid(getuid());
						if (pw && pw->pw_name != entry->value && entry->value != std::to_string(getuid()))
							Utils::error_add(entry->filename, "[" + section + "] " + key + ": cannot drop privileges, not running as root", CRITICAL, entry->line, entry->order + 2);
					}
				}

				return (entry->value);
			}

		#pragma endregion

		#pragma region "Logfile"

			std::string Taskmaster::validate_logfile(const std::string& key, ConfigParser::ConfigEntry *entry, const std::string& dir) {
				if (key.empty() || !entry) return {};

				if (Utils::toUpper(entry->value) != "NONE") {
					entry->value = Utils::expand_path(entry->value, dir);
					std::string default_val = Utils::expand_path(Config.defaultValues[section][key], dir);
					if (!Utils::valid_path(entry->value, dir, false, false, false, true)) {
						Utils::error_add(entry->filename, "[" + section + "] " + key + ": invalid path - " + std::string(strerror(errno)), ERROR, entry->line, entry->order + 2);
						if (entry->value != default_val) {
							Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section][key], WARNING, entry->line, entry->order + 3);
							entry->value = default_val;
							if (!Utils::valid_path(entry->value, dir, false, false, false, true)) {
								Utils::error_add(entry->filename, "[" + section + "] " + key + ": failed to use default value - " + std::string(strerror(errno)), ERROR, entry->line, entry->order + 4);
								entry->value = "NONE";
							}
						} else entry->value = "NONE";
					}
				}

				return (entry->value);
			}

		#pragma endregion

		#pragma region "Size"

			std::string Taskmaster::validate_size(const std::string& key, ConfigParser::ConfigEntry *entry) {
				if (key.empty() || !entry) return {};

				long bytes = Utils::parse_size(entry->value);
				if (bytes == -1 || !Utils::valid_number(std::to_string(bytes), 0, 1024 * 1024 * 1024)) {
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": must be a value between 0 bytes and 1024 MB", ERROR, entry->line, entry->order + 2);
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section][key], WARNING, entry->line, entry->order + 3);
					entry->value = Config.defaultValues[section][key];
				}

				return (entry->value);
			}

		#pragma endregion

		#pragma region "Log Level"

			std::string Taskmaster::validate_loglevel(const std::string& key, ConfigParser::ConfigEntry *entry) {
				if (key.empty() || !entry) return {};

				if (!Utils::valid_loglevel(entry->value)) {
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": must be one of: DEBUG, INFO, WARNING, ERROR, CRITICAL", ERROR, entry->line, entry->order + 2);
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section][key], WARNING, entry->line, entry->order + 3);
					entry->value = Config.defaultValues[section][key];
				}

				return (entry->value);
			}

		#pragma endregion

		#pragma region "Pidfile"

			std::string Taskmaster::validate_pidfile(const std::string& key, ConfigParser::ConfigEntry *entry, const std::string& dir) {
				if (key.empty() || !entry) return {};

				entry->value = Utils::expand_path(entry->value, dir);
				std::string default_val = Utils::expand_path(Config.defaultValues[section][key], dir);
				if (!Utils::valid_path(entry->value, dir)) {
					if (entry->value != default_val) {
						Utils::error_add(entry->filename, "[" + section + "] " + key + ": invalid path - " + std::string(strerror(errno)), ERROR, entry->line, entry->order + 2);
						Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section][key], WARNING, entry->line, entry->order + 3);
						entry->value = default_val;
						if (!Utils::valid_path(entry->value, dir)) {
							Utils::error_add(entry->filename, "[" + section + "] " + key + ": failed to use default value - " + std::string(strerror(errno)), CRITICAL, entry->line, entry->order + 4);
						}
					} else Utils::error_add(entry->filename, "[" + section + "] " + key + ": invalid path - " + std::string(strerror(errno)), CRITICAL, entry->line, entry->order + 2);
				}

				return (entry->value);
			}

		#pragma endregion

		#pragma region "Childlogdir"

			std::string Taskmaster::validate_childlogdir(const std::string& key, ConfigParser::ConfigEntry *entry, const std::string& dir) {
				if (key.empty() || !entry) return {};

				entry->value = Utils::expand_path(entry->value, dir, true, false);
				std::string default_val = Utils::expand_path(Config.defaultValues[section][key], dir);
				if (!Utils::valid_path(entry->value, dir, true)) {
					if (entry->value != default_val) {
						Utils::error_add(entry->filename, "[" + section + "] " + key + ": invalid path - " + std::string(strerror(errno)), ERROR, entry->line, entry->order + 2);
						Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section][key], WARNING, entry->line, entry->order + 3);
						entry->value = default_val;
						if (!Utils::valid_path(entry->value, dir, true)) {
							Utils::error_add(entry->filename, "[" + section + "] " + key + ": failed to use default value - " + std::string(strerror(errno)), ERROR, entry->line, entry->order + 4);
							entry->value = "NONE";
						}
					} else {
						Utils::error_add(entry->filename, "[" + section + "] " + key + ": invalid path - " + std::string(strerror(errno)), ERROR, entry->line, entry->order + 2);
						entry->value = "NONE";
					}
				}
				if (entry->value != "NONE") entry->value = Utils::expand_path(entry->value, dir, true, false);

				return (entry->value);
			}

		#pragma endregion

		#pragma region "Minfds"

			std::string Taskmaster::validate_minfds(const std::string& key, ConfigParser::ConfigEntry *entry) {
				if (key.empty() || !entry) return {};

				if (!Utils::valid_number(entry->value, 1, 65535)) {
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": must be a value between 1 and 65535", ERROR, entry->line, entry->order + 2);
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section][key], WARNING, entry->line, entry->order + 3);
					entry->value = Config.defaultValues[section][key];
				}
				if (Utils::parse_fd_limit(static_cast<uint16_t>(std::stoul(entry->value)))) {
					if (std::stoul(entry->value) > std::stoul(Config.defaultValues[section][key])) {
						Utils::error_add(entry->filename, "[" + section + "] " + key + ": limit could not be applied - system limit too low or insufficient permissions", ERROR, entry->line, entry->order + 4);
						Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section][key], WARNING, entry->line, entry->order + 5);
						entry->value = Config.defaultValues[section][key];
						if (Utils::parse_fd_limit(static_cast<uint16_t>(std::stoul(entry->value)))) {
							Utils::error_add(entry->filename, "[" + section + "] " + key + ": limit could not be applied - system limit too low or insufficient permissions", CRITICAL, entry->line, entry->order + 6);
						}
					} else Utils::error_add(entry->filename, "[" + section + "] " + key + ": limit could not be applied - system limit too low or insufficient permissions", CRITICAL, entry->line, entry->order + 5);
				}

				return (entry->value);
			}

		#pragma endregion

		#pragma region "Minprocs"

			std::string Taskmaster::validate_minprocs(const std::string& key, ConfigParser::ConfigEntry *entry) {
				if (key.empty() || !entry) return {};

				if (!Utils::valid_number(entry->value, 1, 65535)) {
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": must be a value between 1 and 65535", ERROR, entry->line, entry->order + 2);
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section][key], WARNING, entry->line, entry->order + 3);
					entry->value = Config.defaultValues[section][key];
				}
				if (Utils::parse_process_limit(static_cast<uint16_t>(std::stoul(entry->value)))) {
					if (std::stoul(entry->value) > std::stoul(Config.defaultValues[section][key])) {
						Utils::error_add(entry->filename, "[" + section + "] " + key + ": limit could not be applied - system limit too low or insufficient permissions", ERROR, entry->line, entry->order + 4);
						Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section][key], WARNING, entry->line, entry->order + 5);
						entry->value = Config.defaultValues[section][key];
						if (Utils::parse_process_limit(static_cast<uint16_t>(std::stoul(entry->value)))) {
							Utils::error_add(entry->filename, "[" + section + "] " + key + ": limit could not be applied - system limit too low or insufficient permissions", CRITICAL, entry->line, entry->order + 6);
						}
					} else Utils::error_add(entry->filename, "[" + section + "] " + key + ": limit could not be applied - system limit too low or insufficient permissions", CRITICAL, entry->line, entry->order + 5);
				}

				return (entry->value);
			}

		#pragma endregion

	#pragma endregion

	#pragma region "Validate"

		std::string Taskmaster::validate(const std::string& key, ConfigParser::ConfigEntry *entry) {
			if (key.empty() || !entry) return {};

			static std::string dir = "";

			if (key == "directory")			dir = validate_directory(key, entry);

			if (key == "nodaemon")			validate_boolean(key, entry);
			if (key == "silent")			validate_boolean(key, entry);
			if (key == "strip_ansi")		validate_boolean(key, entry);
			if (key == "nocleanup")			validate_boolean(key, entry);

			if (key == "umask")				validate_umask(key, entry);
			if (key == "user")				validate_user(key, entry);

			if (key == "logfile")			validate_logfile(key, entry, dir);
			if (key == "logfile_maxbytes")	validate_size(key, entry);
			if (key == "logfile_backups")	validate_number(key, entry, 0, 1000);
			if (key == "logfile_syslog")	validate_boolean(key, entry);
			if (key == "loglevel")			validate_loglevel(key, entry);

			if (key == "pidfile")			validate_pidfile(key, entry, dir);
			if (key == "childlogdir")		validate_childlogdir(key, entry, dir);

			if (key == "minfds")			validate_minfds(key, entry);
			if (key == "minprocs")			validate_minprocs(key, entry);

			return (entry->value);
		}

	#pragma endregion

	#pragma region "Expand Vars"

		std::string Taskmaster::expand_vars(std::map<std::string, std::string>& env, const std::string& key) {
			if (key.empty()) return {};

			ConfigParser::ConfigEntry *entry = Config.get_value_entry(section, key);
			if (!entry) return {};

			try {
				entry->value = Utils::environment_expand(env, entry->value);
				if (key == "environment") {
					if (!Utils::environment_validate(entry->value)) {
						Utils::error_add(entry->filename, "[" + section + "] " + key + ": invalid variable format", ERROR, entry->line, entry->order + 1);
						entry->value = "";
					} else Utils::environment_add_batch(env, entry->value);
				} else {
					entry->value = Utils::remove_quotes(entry->value);
					if (entry->value.empty() && key != "directory") {
						std::string default_value = Config.defaultValues[section][key];
						if (key == "user" && default_value == "do not switch") default_value = "current user";
						if (key == "logfile" && default_value == "taskmasterd.log") default_value = Utils::expand_path(default_value);
						if (key == "pidfile" && default_value == "taskmasterd.pid") default_value = Utils::expand_path(default_value);
						if (default_value.empty()) default_value = Config.defaultValues[section][key];
						Utils::error_add(entry->filename, "[" + section + "] " + key + ": empty value", ERROR, entry->line, entry->order);
						Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + default_value, WARNING, entry->line, entry->order + 1);
						entry->value = Config.defaultValues[section][key];
					}
				}
			}
			catch(const std::exception& e) {
				Utils::error_add(entry->filename, "[" + section + "] " + key + ": unclosed quote or unfinished escape sequence", ERROR, entry->line, entry->order);
				if (key != "environment") {
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section][key], WARNING, entry->line, entry->order + 1);
					entry->value = Config.defaultValues[section][key];
				} else entry->value = "";
			}

			return (validate(key, entry));
		}

	#pragma endregion

	#pragma region "Initialize"

		void Taskmaster::initialize() {
			std::string configFile;

			ConfigParser::ConfigEntry *entry = Config.get_value_entry(section, "directory");
			if (!entry)	configFile = Utils::expand_path(".", "", true, false);
			else		configFile = entry->filename;

			Utils::environment_initialize(environment);

			std::string HERE			= Utils::environment_get(environment, "HERE");
			std::string HOST_NAME		= Utils::environment_get(environment, "HOST_NAME");

			char hostname[255];
			Utils::environment_add(environment, "HOST_NAME", (!gethostname(hostname, sizeof(hostname))) ? std::string(hostname) : "unknown");
			if (!configFile.empty()) Utils::environment_add(environment, "HERE", std::filesystem::path(configFile).parent_path());

			directory			= expand_vars(environment, "directory");
			nodaemon			= Utils::parse_boolean(expand_vars(environment, "nodaemon"));
			silent				= Utils::parse_boolean(expand_vars(environment, "silent"));
			user				= expand_vars(environment, "user");
			umask				= static_cast<uint16_t>(std::stoi(expand_vars(environment, "umask"), nullptr, 8));
			logfile				= expand_vars(environment, "logfile");
			logfile_maxbytes	= Utils::parse_size(expand_vars(environment, "logfile_maxbytes"));
			logfile_backups		= Utils::parse_number(expand_vars(environment, "logfile_backups"), 0, 1000, 10);
			logfile_syslog		= Utils::parse_boolean(expand_vars(environment, "logfile_syslog"));
			loglevel			= Utils::parse_loglevel(expand_vars(environment, "loglevel"));
			pidfile				= expand_vars(environment, "pidfile");
			childlogdir			= expand_vars(environment, "childlogdir");
			strip_ansi			= Utils::parse_boolean(expand_vars(environment, "strip_ansi"));
			nocleanup			= Utils::parse_boolean(expand_vars(environment, "nocleanup"));
			minfds				= Utils::parse_number(expand_vars(environment, "minfds"), 1, 65535, 1024);
			minprocs			= Utils::parse_number(expand_vars(environment, "minprocs"), 1, 10000, 200);
			identifier			= expand_vars(environment, "identifier");

			if (HERE.empty())		Utils::environment_del(environment, "HERE");
			else					Utils::environment_add(environment, "HERE", HERE);
			if (HOST_NAME.empty())	Utils::environment_del(environment, "HOST_NAME");
			else					Utils::environment_add(environment, "HOST_NAME", HOST_NAME);

			expand_vars(environment, "environment");

			// Initialize Servers
			unix_server.initialize();
			inet_server.initialize();

			// Add Programs
			for (auto& [program, keys] : Config.sections) {
				if (program.substr(0, 8) == "program:") {
					programs.emplace_back(program);
				}
			}

			// Add Groups
			for (auto& [group, keys] : Config.sections) {
				if (group.substr(0, 6) == "group:") {
					bool add_group = false;
					ConfigParser::ConfigEntry *g_entry = Config.get_value_entry(group, "programs");
					if (g_entry->value.empty()) continue;

					std::vector<std::string> program_names = Utils::toVector(g_entry->value, ",");

					for (const auto& program_name : program_names) {
						auto it = std::find_if(programs.begin(), programs.end(), [&program_name](const auto& program) { return (program.name == program_name); });

						if		(it == programs.end())	Utils::error_add(g_entry->filename, "[" + group + "] programs: program '" + program_name + "' not found", ERROR, g_entry->line, g_entry->order);
						else if	(!it->disabled)			add_group = true;
					}

					if (add_group) groups.emplace_back(group);
				}
			}
		}

	#pragma endregion

	#pragma region "Reload"

		void Taskmaster::reload() {
			reload_programs.clear();
			groups.clear();

			for (auto& [program, keys] : Config.sections) {
				if (program.substr(0, 8) == "program:") {
					reload_programs.emplace_back(program);
				}
			}

			for (auto& [group, keys] : Config.sections) {
				if (group.substr(0, 6) == "group:") {
					bool add_group = false;
					ConfigParser::ConfigEntry *g_entry = Config.get_value_entry(group, "programs");
					if (g_entry->value.empty()) continue;

					std::vector<std::string> program_names = Utils::toVector(g_entry->value, ",");

					for (const auto& program_name : program_names) {
						auto it = std::find_if(reload_programs.begin(), reload_programs.end(), [&program_name](const auto& program) { return (program.name == program_name); });

						if		(it == reload_programs.end())	Utils::error_add(g_entry->filename, "[" + group + "] programs: program '" + program_name + "' not found", ERROR, g_entry->line, g_entry->order);
						else if	(!it->disabled)			add_group = true;
					}

					if (add_group) groups.emplace_back(group);
				}
			}
		}

	#pragma endregion

#pragma endregion

#pragma region "Reload"

	#pragma region "Has Changes"

		bool Taskmaster::has_changes(const Program& old_prog, const Program& new_prog) {
			// Comparar campos básicos del programa
			if (old_prog.name != new_prog.name ||
				old_prog.numprocs != new_prog.numprocs ||
				old_prog.numprocs_start != new_prog.numprocs_start ||
				old_prog.disabled != new_prog.disabled) {
				return true;
			}

			// Si no hay procesos en alguno de los programas, hay cambios
			if (old_prog.process.empty() || new_prog.process.empty()) {
				return old_prog.process.size() != new_prog.process.size();
			}

			// Comparar solo el primer proceso
			const auto& old_proc = old_prog.process[0];
			const auto& new_proc = new_prog.process[0];

			// Cambios críticos que requieren reinicio
			return (old_proc.command != new_proc.command ||
					old_proc.arguments != new_proc.arguments ||
					old_proc.directory != new_proc.directory ||
					old_proc.user != new_proc.user ||
					old_proc.umask != new_proc.umask ||
					old_proc.environment != new_proc.environment ||
					old_proc.tty_mode != new_proc.tty_mode ||
					old_proc.redirect_stderr != new_proc.redirect_stderr ||
					old_proc.autostart != new_proc.autostart ||
					old_proc.autorestart != new_proc.autorestart ||
					old_proc.startsecs != new_proc.startsecs ||
					old_proc.startretries != new_proc.startretries ||
					old_proc.exitcodes != new_proc.exitcodes ||
					old_proc.stopsignal != new_proc.stopsignal ||
					old_proc.stopwaitsecs != new_proc.stopwaitsecs ||
					old_proc.stopasgroup != new_proc.stopasgroup ||
					old_proc.killasgroup != new_proc.killasgroup ||
					old_proc.stdout_logfile != new_proc.stdout_logfile ||
					old_proc.stdout_logfile_maxbytes != new_proc.stdout_logfile_maxbytes ||
					old_proc.stdout_logfile_backups != new_proc.stdout_logfile_backups ||
					old_proc.stdout_logfile_syslog != new_proc.stdout_logfile_syslog ||
					old_proc.stderr_logfile != new_proc.stderr_logfile ||
					old_proc.stderr_logfile_maxbytes != new_proc.stderr_logfile_maxbytes ||
					old_proc.stderr_logfile_backups != new_proc.stderr_logfile_backups ||
					old_proc.stderr_logfile_syslog != new_proc.stderr_logfile_syslog ||
					old_proc.serverurl != new_proc.serverurl ||
					old_proc.priority != new_proc.priority);
		}

	#pragma endregion

	#pragma region "Update Programs"

		void Taskmaster::update_programs(Program& existing_prog, Program&& new_prog) {
			std::vector<Process> runtime_backup;
			for (const auto& proc : existing_prog.process) {
				Process backup;
				backup.pid = proc.pid;
				backup.status = proc.status;
				backup.start_time = proc.start_time;
				backup.stop_time = proc.stop_time;
				backup.change_time = proc.change_time;
				backup.uptime = proc.uptime;
				backup.process_num = proc.process_num;
				backup.name = proc.name;
				runtime_backup.push_back(backup);
			}

			existing_prog.section = std::move(new_prog.section);
			existing_prog.name = std::move(new_prog.name);
			existing_prog.numprocs = new_prog.numprocs;
			existing_prog.numprocs_start = new_prog.numprocs_start;
			existing_prog.disabled = new_prog.disabled;
			existing_prog.groups = std::move(new_prog.groups);
			existing_prog.process = std::move(new_prog.process);

			for (auto& new_proc : existing_prog.process) {
				for (const auto& backup : runtime_backup) {
					if (new_proc.name == backup.name || new_proc.process_num == backup.process_num) {
						new_proc.pid = backup.pid;
						new_proc.status = backup.status;
						new_proc.start_time = backup.start_time;
						new_proc.stop_time = backup.stop_time;
						new_proc.change_time = backup.change_time;
						new_proc.uptime = backup.uptime;
						break;
					}
				}
			}
		}

	#pragma endregion

	#pragma region "Process Restarts"

		void Taskmaster::process_restarts() {
			for (auto& program : programs) {
				if (program.needs_restart) {
					// parada y reinicio de procesos
					Log.info("process restart for program: " + program.name);
					program.needs_restart = false;
				}
			}
		}

	#pragma endregion

	#pragma region "Process Reload"

		void Taskmaster::process_reload() {
			bool is_restart = false;

			auto programs_it = programs.begin();
			while (programs_it != programs.end()) {
				bool found = false;
				for (auto& rld_program : reload_programs) {
					if (programs_it->name == rld_program.name) {
						found = true;
						// Hay cambios en un programa
						if (has_changes(*programs_it, rld_program)) {
							programs_it->needs_restart = true;
							is_restart = true;
							Log.info("Program '" + programs_it->name + "' marked for restart due to config changes");
						}
						update_programs(*programs_it, std::move(rld_program)); break;
					}
				}
				if (!found) {
					// El programa se ha eliminado
					Log.info("Program '" + programs_it->name + "' removed");
					// si esta en ejecucion, detener antes de borrar
					programs_it = programs.erase(programs_it);
				} else	programs_it++;
			}

			// Hay programas nuevos
			for (auto& rld_program : reload_programs) {
				bool found = false;
				for (const auto& program : programs) {
					if (rld_program.name == program.name) { found = true; break; }
				}
				if (!found) {
					// Programa nuevo, añadirlo
					Log.info("New program '" + rld_program.name + "' added");
					programs.emplace_back(std::move(rld_program));
				}
			}

			// Si hay cambios críticos, programar restart de procesos afectados
			if (is_restart) process_restarts();
			
			reload_programs.clear();
		}

	#pragma endregion

#pragma endregion

#pragma region "Clean Up"

	void Taskmaster::cleanup(bool silent) {
		unix_server.close();
		inet_server.close();
		pidlock.unlock();
		epoll.close();
		Signal::close();
		if (!silent) Log.info("Taskmasterd: closed\n");
		Log.close();
	}

#pragma endregion

#pragma region "Daemonize"

	int Taskmaster::daemonize() {

		if (!nodaemon) {
			// 1. fork()
			int pid = fork();
			if (pid < 0) {
				Log.critical("Daemon: first fork failed - " + std::string(strerror(errno)));
				return (1);
			}
			if (pid > 0) return (-1);
			Log.debug("Daemon: first fork completed");

			// 2. setsid()
			if (setsid() < 0) {
				Log.critical("Daemon: setsid() failed - " + std::string(strerror(errno)));
				return (1);
			}
			Log.debug("Daemon: setsid completed");

			// 3. fork()
			pid = fork();
			if (pid < 0) {
				Log.critical("Daemon: second fork failed - " + std::string(strerror(errno)));
				return (1);
			}
			if (pid > 0) return (-1);
			Log.debug("Daemon: second fork completed");

			// 4. close()
			close(0); close(1); close(2);
			Log.debug("Daemon: standard file descriptors closed");

			Log.debug("Daemon: completed successfully");
		}

		// 5. umask()
		::umask(umask);
		std::ostringstream oss;
		oss << "Taskmasterd: umask set to " << std::oct << std::setw(3) << std::setfill('0') << umask;
		Log.debug(oss.str());

		// 6. chdir()
		if (chdir("/"))					Log.warning("Taskmasterd: failed to change directory to / - " + std::string(strerror(errno)));
		else							Log.debug("Taskmasterd: working directory set to /");

		// 7. flock()
		if (pidlock.lock(pidfile)) return (1);

		pid = getpid();

		return (0);
	}

#pragma endregion
