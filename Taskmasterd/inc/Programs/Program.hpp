/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Program.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/02 17:24:36 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/08 20:28:37 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include "Programs/Process.hpp"

	#include <string>															// std::string
	#include <cstdint>															// uint16_t
	#include <climits>															// LONG_MIN, LONG_MAX
	#include <map>																// std::map
	#include <vector>															// std::vector

#pragma endregion

#pragma region "Program"

	class Group;
	class Program {

		public:

			// Constructors
			Program(const std::string _name);
			Program(const Program&) = default;
			Program(Program&&) = default;
			~Program() = default;

			// Overloads
			Program& operator=(const Program&) = default;
			Program& operator=(Program&&) = default;

			// Variables
			std::string					section;
			std::string					name;
			uint16_t        			numprocs;
			uint16_t        			numprocs_start;
			bool						disabled;
			bool						needs_restart;
			std::vector<Process>		process;
			std::vector<std::string>	groups;

		private:

			std::string	validate_directory(const std::string& key, ConfigParser::ConfigEntry *entry);
			std::string	validate_boolean(const std::string& key, ConfigParser::ConfigEntry *entry);
			std::string	validate_number(const std::string& key, ConfigParser::ConfigEntry *entry, long min = 0, long max = LONG_MAX);
			std::string	validate_autorestart(const std::string& key, ConfigParser::ConfigEntry *entry);
			std::string	validate_exitcodes(const std::string& key, ConfigParser::ConfigEntry *entry);
			std::string	validate_stopsignal(const std::string& key, ConfigParser::ConfigEntry *entry);
			std::string	validate_umask(const std::string& key, ConfigParser::ConfigEntry *entry);
			std::string	validate_user(const std::string& key, ConfigParser::ConfigEntry *entry);
			std::string	validate_logfile(const std::string& key, ConfigParser::ConfigEntry *entry, const std::string& dir);
			std::string	validate_size(const std::string& key, ConfigParser::ConfigEntry *entry);
			std::string	validate_serverurl(const std::string& key, ConfigParser::ConfigEntry *entry);

			std::string	validate(const std::string& key, ConfigParser::ConfigEntry *entry);
			std::string	expand_vars(std::map<std::string, std::string>& env, const std::string& key);
			void		add_groups(std::map<std::string, std::string>& env, std::string& configFile, uint16_t order);
			void		initialize();

	};

#pragma endregion
