/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TaskManager.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/02 17:24:36 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/07 13:41:35 by vzurera-         ###   ########.fr       */
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
	#include <vector>															// std::vector

#pragma endregion

#pragma region "TaskManager"

	class TaskManager {

		public:

			// Constructors
			TaskManager() = default;
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

			std::vector<Program>				reload_programs;
			std::vector<Program>				programs;
			std::vector<Group>					groups;
			UnixServer							unix_server;
			InetServer							inet_server;

			std::string	validate(const std::string& key, ConfigParser::ConfigEntry *entry);
			std::string	expand_vars(std::map<std::string, std::string>& env, const std::string& key);
			void		initialize();
			void		reload();
			void		process_reload();

		private:

			bool		has_changes(const Program& old_prog, const Program& new_prog);
			void		update_program(Program& existing_prog, Program&& new_prog);
			void		process_restarts();

	};

#pragma endregion
	
#pragma region "Variables"

	extern TaskManager TaskMaster;

#pragma endregion
