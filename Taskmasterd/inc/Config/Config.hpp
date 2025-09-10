/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/24 21:47:27 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/10 18:35:28 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include "Utils/Utils.hpp"
	#include "Config/Options.hpp"

	#include <cstdint>															// uint16_t, uint32_t
	#include <set>																// std::set
	#include <map>																// std::map
	#include <vector>															// std::vector

#pragma endregion

#pragma region "ConfigParser"

	class ConfigParser {

		public:

			// Constructors
			ConfigParser();
			ConfigParser(const ConfigParser&) = default;
			ConfigParser(ConfigParser&&) = default;
			~ConfigParser() = default;

			// Overloads
			ConfigParser& operator=(const ConfigParser&) = default;
			ConfigParser& operator=(ConfigParser&&) = default;

			// Structures
			struct ConfigEntry {
				std::string	value;
				std::string	filename;
				uint16_t	line;
				uint32_t	order;
			};

			// Variables
			std::map<std::string, std::map<std::string, ConfigEntry>>	sections;
			std::map<std::string, std::map<std::string, std::string>>	defaultValues;
			bool														is_root;
			

			// Keys
			ConfigEntry*	get_value_entry(const std::string& section, const std::string& key);

			// Section
			bool			has_section(const std::string& section) const;

			// Load
			int				load(int argc, char **argv);
			int				reload();

		private:

			// Variables
			uint16_t													order;
			std::map<std::string, std::set<std::string>>				validKeys;
			std::set<std::string>										validSections;
			std::string													currentSection;
			std::string													mainConfigFile;
			bool														in_environment;
			bool														in_reloading;

			// Include
			int							include_load_file(const std::string& filePath);
			std::vector<std::string>	include_parse_files(const std::string& fileString, const std::string& configFile);
			int							include_process(std::string& ConfigFile);

			// Keys
			std::string					get_value(const std::string& section, const std::string& key) const;
			bool						key_valid(const std::string& section, const std::string& key) const;
			int							key_parse(const std::string& line, int line_number, std::string& filename, bool start_space);

			// Section
			bool						is_section(const std::string& line) const;
			std::string					section_type(const std::string& section) const;
			std::string					section_extract(const std::string& line) const;
			int							section_parse(const std::string& line, int line_number, std::string& filename);

			// Load
			void						merge_options(const ConfigOptions& Options);
			void						load_file(const std::string& filePath = "");

			// Initialize
			void						initialize();
			void						default_values();

	};

#pragma endregion
	
#pragma region "Variables"

	extern ConfigParser Config;

#pragma endregion
