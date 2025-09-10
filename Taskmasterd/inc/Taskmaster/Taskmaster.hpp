/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Taskmaster.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/02 17:24:36 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/10 18:33:38 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include "Utils/Utils.hpp"
	#include "Config/Config.hpp"
	#include "Logging/TaskmasterLog.hpp"
	#include "Taskmaster/Pidlock.hpp"
	#include "Programs/Program.hpp"
	#include "Programs/Group.hpp"
	#include "Servers/UnixServer.hpp"
	#include "Servers/InetServer.hpp"
	#include "Loop/Epoll.hpp"
	#include "Loop/Signal.hpp"

	#include <cstdint>															// uint8_t, uint16_t
	#include <climits>															// LONG_MIN, LONG_MAX
	#include <vector>															// std::vector

#pragma endregion

#pragma region "Taskmaster"

	class Taskmaster {

		public:

			// Constructors
			Taskmaster();
			Taskmaster(const Taskmaster&) = delete;
			Taskmaster(Taskmaster&&) = delete;
			~Taskmaster() = default;

			// Overloads
			Taskmaster& operator=(const Taskmaster&) = delete;
			Taskmaster& operator=(Taskmaster&&) = delete;

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

			bool								running;
			pid_t								pid;
			Pidlock								pidlock;
			Epoll								epoll;

			std::vector<Program>				programs;
			std::vector<Group>					groups;
			std::vector<Program>				reload_programs;
			UnixServer							unix_server;
			InetServer							inet_server;

			void		initialize();
			void		reload();
			void		process_reload();
			void		clean_up(bool silent = false);
			int			daemonize();

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

	extern Taskmaster tskm;

#pragma endregion
