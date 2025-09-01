/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Initialize.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 11:38:04 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/01 22:06:33 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Config.hpp"

#pragma endregion

#pragma region "Variables"

	ConfigParser Config;

#pragma endregion

#pragma region "Constructors"

	ConfigParser::ConfigParser() {
		initialize();
		default_values();
	}

#pragma endregion

#pragma region "Initialize"

	void ConfigParser::initialize() {
		validSections = {
			"taskmasterd",
			"program:",
			"group:",
			"unix_http_server",
			"inet_http_server",
			"include",
			"taskmasterctl"
		};

		validKeys = {
			{"taskmasterd", {
				"nodaemon",
				"silent",
				"user",
				"umask",
				"directory",
				"logfile",
				"logfile_maxbytes",
				"logfile_backups",
				"logfile_syslog",
				"loglevel",
				"pidfile",
				"identifier",
				"childlogdir",
				"strip_ansi",
				"nocleanup",
				"minfds",
				"minprocs",
				"environment"
			}},

			{"program:", {
				"command",
				"process_name",
				"tty_mode",
				"numprocs",
				"directory",
				"umask",
				"priority",
				"autostart",
				"autorestart",
				"startsecs",
				"startretries",
				"exitcodes",
				"stopsignal",
				"stopwaitsecs",
				"stopasgroup",
				"killasgroup",
				"user",
				"redirect_stderr",
				"stdout_logfile",
				"stdout_logfile_maxbytes",
				"stdout_logfile_backups",
				"stdout_logfile_syslog",
				"stderr_logfile",
				"stderr_logfile_maxbytes",
				"stderr_logfile_backups",
				"stderr_logfile_syslog",
				"serverurl",
				"environment"
			}},

			{"group:", {
				"programs",
				"priority"
			}},

			{"include", {
				"files"
			}},

			{"unix_http_server", {
				"file",
				"chmod",
				"chown",
				"username",
				"password"
			}},

			{"inet_http_server", {
				"port",
				"username",
				"password"
			}}
		};
	}

#pragma endregion

#pragma region "Default Values"

	void ConfigParser::default_values() {
		std::string	childlogdir = temp_path();

		defaultValues = {
			{"taskmasterd", {
				{"nodaemon", "false"},
				{"silent", "false"},
				{"user", "do not switch"},
				{"umask", "022"},
				{"directory", "do not change"},
				{"logfile", "logfile.log"},
				{"logfile_maxbytes", "50MB"},
				{"logfile_backups", "10"},
				{"logfile_syslog", "false"},
				{"loglevel", "DEBUG"},
				{"pidfile", "taskmasterd.pid"},
				{"identifier", "taskmaster"},
				{"childlogdir", childlogdir},
				{"strip_ansi", "false"},
				{"nocleanup", "false"},
				{"minfds", "1024"},
				{"minprocs", "200"},
				{"environment", ""}
			}},

			{"program:", {
				{"tty_mode", "false"},
				{"numprocs", "1"},
				{"priority", "999"},
				{"autostart", "true"},
				{"autorestart", "unexpected"},
				{"startsecs", "1"},
				{"startretries", "3"},
				{"exitcodes", "0,2"},
				{"stopsignal", "TERM"},
				{"stopwaitsecs", "10"},
				{"stopasgroup", "false"},
				{"killasgroup", "false"},
				{"user", "do not switch"},
				{"directory", "do not change"},
				{"umask", "inherit"},
				{"redirect_stderr", "false"},
				{"stdout_logfile", "AUTO"},
				{"stdout_logfile_maxbytes", "50MB"},
				{"stdout_logfile_backups", "10"},
				{"stdout_logfile_syslog", "false"},
				{"stderr_logfile", "AUTO"},
				{"stderr_logfile_maxbytes", "50MB"},
				{"stderr_logfile_backups", "10"},
				{"stderr_logfile_syslog", "false"},
				{"serverurl", "AUTO"},
				{"environment", ""},
			}},

			{"group:", {
				{"priority", "999"}
			}},

			{"unix_http_server", {
				{"chmod", "0700"},
				{"chown", ""},
				{"username", ""},
				{"password", ""}
			}},

			{"inet_http_server", {
				{"username", ""},
				{"password", ""}
			}}
		};
	}

#pragma endregion
