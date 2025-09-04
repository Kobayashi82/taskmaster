/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Program.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/02 17:23:05 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/04 11:54:08 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Utils/Utils.hpp"
	#include "Config/Config.hpp"
	#include "Programs/Program.hpp"

	#include <sstream>															// std::stringstream

#pragma endregion

#pragma region "Constructors"

	Program::Program(const std::string _section) : section(_section), name(_section.substr(8)) {
		try {
			numprocs		= std::atoi(Config.get_value(section, "numprocs").c_str());
			numprocs_start	= std::atoi(Config.get_value(section, "numprocs_start").c_str());
		} catch(const std::exception& e) {
			numprocs = 1;
			numprocs_start = 0;
		}
		add_process();
	}

#pragma endregion

	void Program::add_process() {
		std::map<std::string, std::string>	environment;

		Utils::environment_initialize(environment);
		Utils::environment_add_batch(environment, Config.get_value("taskmasterd", "environment"));
		Utils::environment_add(environment, "SUPERVISOR_ENABLED", "1");
		// Si pertenece a grupos, ponerlo aqui?
		Utils::environment_add(environment, "SUPERVISOR_PROCESS_GROUP", "");

		std::string	serverulr		= Config.get_value(section, "serverulr");
		if (!serverulr.empty() && Utils::toUpper(serverulr) != "AUTO")	Utils::environment_add(environment, "SUPERVISOR_SERVER_URL", serverulr);
		else															Utils::environment_del(environment, "SUPERVISOR_SERVER_URL");

		uint16_t current_process = 0;
		uint16_t current_process_num = numprocs_start;
		process.reserve(numprocs);

		while (current_process < numprocs) {
			try {
				Process proc;

				std::map<std::string, std::string> temp_vars;
				Utils::environment_add(temp_vars, "EX_PROCESS_NUM", std::to_string(current_process_num++));
				// expandir EX_PROCESS_NUM en current_process_num

				proc.name = Config.get_value(section, "process_name");
				proc.command = Config.get_value(section, "command");
				proc.directory = Config.get_value(section, "directory");

				std::string umask = Config.get_value(section, "umask");
				if (umask.empty() || umask == "inherit") umask = Config.get_value("taskmasterd", "umask");

				proc.umask = static_cast<uint16_t>(std::stoi(umask, nullptr, 8));
				proc.priority = static_cast<uint16_t>(std::atoi(Config.get_value(section, "priority").c_str()));
				proc.autostart = Utils::parse_bool(Config.get_value(section, "autostart"));
				proc.autorestart = Utils::parse_bool(Config.get_value(section, "autorestart"), true);
				proc.startsecs = static_cast<uint16_t>(std::atoi(Config.get_value(section, "startsecs").c_str()));
				proc.startretries = static_cast<uint8_t>(std::atoi(Config.get_value(section, "startretries").c_str()));

				std::stringstream ss(Config.get_value(section, "exitcodes")); std::string token;
				while (std::getline(ss, token, ',')) proc.exitcodes.push_back(static_cast<uint8_t>(std::stoi(token)));

				proc.stopsignal = Utils::parse_signal(Config.get_value(section, "stopsignal"));
				proc.stopwaitsecs = static_cast<uint16_t>(std::atoi(Config.get_value(section, "stopwaitsecs").c_str()));
				proc.stopasgroup = Utils::parse_bool(Config.get_value(section, "stopasgroup"));
				proc.killasgroup = Utils::parse_bool(Config.get_value(section, "killasgroup"));
				proc.tty_mode = Utils::parse_bool(Config.get_value(section, "tty_mode"));
				proc.user = Config.get_value(section, "user");
				proc.redirect_stderr = Utils::parse_bool(Config.get_value(section, "redirect_stderr"));
				proc.stdout_logfile = Config.get_value(section, "stdout_logfile");
				proc.stdout_logfile_maxbytes = Utils::parse_size(Config.get_value(section, "stdout_logfile_maxbytes"));
				proc.stdout_logfile_backups = static_cast<uint16_t>(std::atoi(Config.get_value(section, "stdout_logfile_backups").c_str()));
				proc.stdout_logfile_syslog = Utils::parse_bool(Config.get_value(section, "stdout_logfile_syslog"));
				proc.stderr_logfile = Config.get_value(section, "stderr_logfile");
				proc.stderr_logfile_maxbytes = Utils::parse_size(Config.get_value(section, "stderr_logfile_maxbytes"));
				proc.stderr_logfile_backups = static_cast<uint16_t>(std::atoi(Config.get_value(section, "stderr_logfile_backups").c_str()));
				proc.stderr_logfile_syslog = Utils::parse_bool(Config.get_value(section, "stderr_logfile_syslog"));

				Utils::environment_clone(proc.environment, environment);
				Utils::environment_add(proc.environment, "SUPERVISOR_PROCESS_NAME", "");
				Utils::environment_add_batch(proc.environment, Config.get_value(section, "environment"));

				proc.process_num = current_process;
				proc.program_name = name;
				proc.program = this;

				process.push_back(proc);
				current_process++;
			} catch(const std::exception& e) {
				disabled = true;
				return;
			}
		}
	}
