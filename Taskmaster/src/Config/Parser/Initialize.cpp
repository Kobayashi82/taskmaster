/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Initialize.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 11:38:04 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/27 12:03:46 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Parser.hpp"

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
				"stderr_logfile",
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
		std::string	logfile = expand_path("taskmasterd.log");	// $CWD/taskmasterd.log
		std::string	pidfile = expand_path("taskmasterd.pid");	// $CWD/taskmasterd.pid
		std::string	childlogdir = temp_path();
		std::string	logfile_maxbytes = std::to_string(50 * 1024 * 1024);	// 50 MB

		defaultValues = {
			{"taskmasterd", {
				{"nodaemon", "false"},
				{"silent", "false"},
				{"user", "do not switch"},
				{"umask", "022"},
				{"directory", "do not change"},
				{"logfile", logfile},
				{"logfile_maxbytes", logfile_maxbytes},
				{"logfile_backups", "10"},
				{"loglevel", "1"},
				{"pidfile", pidfile},
				{"identifier", "taskmaster"},
				{"childlogdir", childlogdir},
				{"strip_ansi", "false"},
				{"nocleanup", "false"},
				{"minfds", "1024"},
				{"minprocs", "200"},
				{"environment", ""}
			}},

			{"program:", {
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
				{"umask", "022"},
				{"redirect_stderr", "false"},
				{"stdout_logfile", "AUTO"},
				{"stderr_logfile", "AUTO"},
				{"environment", ""}
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
