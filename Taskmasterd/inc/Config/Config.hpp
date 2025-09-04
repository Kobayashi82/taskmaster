/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/24 21:47:27 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/04 14:08:59 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include "Config/Options.hpp"

	#include <cstdint>															// uint16_t
	#include <set>																// std::set
	#include <map>																// std::map
	#include <vector>															// std::vector
	#include <climits>															// LONG_MIN, LONG_MAX

#pragma endregion

#pragma region "ConfigParser"

	class ConfigParser {

		public:

			// Structures
			struct ConfigEntry {
				std::string	value;
				std::string	filename;
				uint16_t	line;
				uint16_t	order;
			};

			// Variables
			std::map<std::string, std::map<std::string, ConfigEntry>>	sections;
			bool														is_root;

			// Constructors
			ConfigParser();
			ConfigParser(const ConfigParser&) = delete;
			ConfigParser(ConfigParser&&) = default;
			~ConfigParser() = default;

			// Overloads
			ConfigParser& operator=(const ConfigParser&) = delete;
			ConfigParser& operator=(ConfigParser&&) = delete;

			// Keys
			ConfigEntry*	get_value_entry(const std::string& section, const std::string& key);
			std::string		get_value(const std::string& section, const std::string& key) const;

			// Section
			void			print() const;
			bool			has_section(const std::string& section) const;

			// Parser
			int				load(int argc, char **argv);

			// Validation
			int				validate_options(ConfigOptions& Options) const;

		private:

			// Variables
			uint16_t													order;
			std::map<std::string, std::set<std::string>>				validKeys;
			std::set<std::string>										validSections;
			std::map<std::string, std::map<std::string, std::string>>	defaultValues;
			std::string													currentSection;

			// Include
			int							include_load_file(const std::string& filePath);
			std::vector<std::string>	include_parse_files(const std::string& fileString, const std::string& configFile);
			void						include_process(std::string& ConfigFile);

			// Keys
			bool						key_valid(const std::string& section, const std::string& key) const;
			int							key_parse(const std::string& line, int line_number, std::string& filename);

			// Section
			bool						is_section(const std::string& line) const;
			std::string					section_type(const std::string& section) const;
			std::string					section_extract(const std::string& line) const;
			int							section_parse(const std::string& line, int line_number, std::string& filename);

			// Initialize
			void						initialize();
			void						default_values();

			// Parser
			void						merge_options(const ConfigOptions& Options);
			void						load_file(const std::string& filePath = "");

			// Validation
			bool						valid_bool(const std::string& value) const;
			bool						valid_number(const std::string& value, long min = 0, long max = LONG_MAX) const;
			bool						valid_path(const std::string& value, const std::string current_path = "", bool is_directory = false, bool allow_auto = false, bool allow_none = false) const;
			bool						valid_signal(const std::string& value) const;
			bool						valid_code(const std::string& value) const;
			bool						valid_loglevel(const std::string& value) const;
			bool						valid_autorestart(const std::string& value) const;
			bool						valid_umask(const std::string& value) const;
			bool						valid_user(const std::string& value) const;
			bool						valid_chown(const std::string& value) const;
			bool						valid_password(const std::string& value) const;
			bool						valid_port(const std::string& value) const;
			bool						valid_serverurl(const std::string &value) const;

			void						validate_taskmasterd();
			void						validate_program();
			void						validate_group();
			void						validate_unix_server();
			void						validate_inet_server();
			void						validate();

	};

#pragma endregion
	
#pragma region "Variables"

	extern ConfigParser Config;

#pragma endregion
