/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TaskManager.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/02 17:24:36 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/08 20:35:20 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include "Config/Config.hpp"
	#include "Programs/Program.hpp"
	#include "Programs/Group.hpp"
	#include "Programs/UnixServer.hpp"
	#include "Programs/InetServer.hpp"

	#include <cstdint>															// uint8_t, uint16_t
	#include <climits>															// LONG_MIN, LONG_MAX
	#include <vector>															// std::vector

#pragma endregion

#pragma region "TaskManager"

	class TaskManager {

		public:

			// Constructors
			TaskManager();
			TaskManager(const TaskManager&) = delete;
			~TaskManager() = default;

			// Overloads
			TaskManager& operator=(const TaskManager&) = delete;

			// Variables
			bool								nodaemon;
			bool								silent;
			std::string							user;
			uint16_t							umask;
			std::string							directory;
			std::string							logfile;
			uint32_t							logfile_maxbytes;
			uint16_t							logfile_backups;
			bool								logfile_syslog;
			uint8_t								loglevel;
			std::string							pidfile;
			std::string							identifier;
			std::string							childlogdir;
			bool								strip_ansi;
			bool								nocleanup;
			uint16_t							minfds;
			uint16_t							minprocs;
			std::map<std::string, std::string>	environment;

			std::string							section;
			std::vector<Program>				reload_programs;
			std::vector<Program>				programs;
			std::vector<Group>					groups;
			UnixServer							unix_server;
			InetServer							inet_server;

			void		initialize();
			void		reload();
			void		process_reload();

		private:

			std::string	validate_directory(const std::string& key, ConfigParser::ConfigEntry *entry);
			std::string	validate_boolean(const std::string& key, ConfigParser::ConfigEntry *entry);
			std::string	validate_number(const std::string& key, ConfigParser::ConfigEntry *entry, long min = 0, long max = LONG_MAX);
			std::string	validate_umask(const std::string& key, ConfigParser::ConfigEntry *entry);
			std::string	validate_user(const std::string& key, ConfigParser::ConfigEntry *entry);
			std::string	validate_logfile(const std::string& key, ConfigParser::ConfigEntry *entry, const std::string& dir);
			std::string	validate_size(const std::string& key, ConfigParser::ConfigEntry *entry);
			std::string	validate_loglevel(const std::string& key, ConfigParser::ConfigEntry *entry);
			std::string	validate_pidfile(const std::string& key, ConfigParser::ConfigEntry *entry, const std::string& dir);
			std::string	validate_childlogdir(const std::string& key, ConfigParser::ConfigEntry *entry, const std::string& dir);
			std::string	validate_minfds(const std::string& key, ConfigParser::ConfigEntry *entry);
			std::string	validate_minprocs(const std::string& key, ConfigParser::ConfigEntry *entry);

			std::string	validate(const std::string& key, ConfigParser::ConfigEntry *entry);
			std::string	expand_vars(std::map<std::string, std::string>& env, const std::string& key);

			bool		has_changes(const Program& old_prog, const Program& new_prog);
			void		update_programs(Program& existing_prog, Program&& new_prog);
			void		process_restarts();

	};

#pragma endregion
	
#pragma region "Variables"

	extern TaskManager TaskMaster;

#pragma endregion
