/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/24 21:47:27 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/27 12:19:50 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include "Config/Options.hpp"

	#include <cstdint>															// uint16_t
	#include <string>															// std::string
	#include <set>																// std::set
	#include <map>																// std::map
	#include <vector>															// std::vector
	#include <climits>															// LONG_MIN, LONG_MAX

#pragma endregion

#pragma region "ConfigParser"

	class ConfigParser {

		private:

			// Variables
			bool														in_include;
			bool														section_on_error;
			std::string													currentSection;
			std::map<std::string, std::map<std::string, std::string>>	sections;
			std::set<std::string>										validSections;
			std::map<std::string, std::set<std::string>>				validKeys;
			std::map<std::string, std::map<std::string, std::string>>	defaultValues;

			// Initialize
			void		initialize();
			void		default_values();

			// Key=Value
			bool		isValidKey(const std::string& section, const std::string& key) const;
			void		parseKeyValue(const std::string& line);

			// Section
			std::string	SectionType(const std::string& section) const;
			bool		isValidSection(const std::string& section) const;
			std::string	extractSection(const std::string& line) const;
			void		parseSection(const std::string& line);

			// Line
			bool		isSection(const std::string& line) const;
			bool		isComment(const std::string& line) const;
			void		parseLine(const std::string& line);

			// Validation helpers
			bool		isValidBool(const std::string& value) const;
			bool		isValidNumber(const std::string& value, long min = 0, long max = 2147483647) const;
			bool		isValidPath(const std::string& value, bool is_directory) const;
			bool		isValidSignal(const std::string& value) const;
			bool		isValidExitCodes(const std::string& value) const;
			bool		isValidLogLevel(const std::string& value) const;
			bool		isValidAutorestart(const std::string& value) const;
			bool		isValidUmask(const std::string& value) const;
			bool		isValidUser(const std::string& value) const;

			// Section validators
			void		validateTaskmasterdSection(const std::string& section, const std::string& key, std::string& value) const;
			void		validateProgramSection(const std::string& section, std::string& key, std::string& value) const;
			void		validateUnixHttpServerSection(const std::string& section, std::string& key, std::string& value) const;
			void		validateInetHttpServerSection(const std::string& section, std::string& key, std::string& value) const;
			void		validateGroupSection(const std::string& section, std::string& key, std::string& value) const;

			void		parseIncludeFile(const std::string& filePath);
			void		parseProcessInclude();

		public:

			// Constructors
			ConfigParser();
			ConfigParser(const ConfigParser&) = delete;
			~ConfigParser() = default;

			// Overloads
			ConfigParser& operator=(const ConfigParser&) = delete;

			// File
			void	parseFile(const std::string& filePath);
			void	validate(const std::string& section, std::string& key, std::string& value) const;

			// Getters
			bool								hasSection(const std::string& section) const;
			std::string							getValue(const std::string& section, const std::string& key, const std::string& defaultValue = "") const;
			std::map<std::string, std::string>	getSection(const std::string& section) const;
			std::map<std::string, std::string>	getSectionWithDefaults(const std::string& section) const;
			std::vector<std::string>			getProgramSections() const;
			std::vector<std::string>			getGroupSections() const;

			// Utils
			std::string	trim(const std::string& str) const;
			std::string	toLower(const std::string& str) const;
			std::string	toUpper(const std::string& str) const;
			std::string	expand_path(const std::string& path) const;
			std::string	temp_path() const;
			std::string	config_path() const;
			int			check_fd_limit(uint16_t minfds) const;
			int			check_process_limit(uint16_t minprocs) const;
			long		parse_size(const std::string &value) const;

			// Debug
			void	print() const;

			void	add_opt_args(ConfigOptions& Options);
	};

#pragma endregion
	
#pragma region "Variables"

	extern ConfigParser Parser;

#pragma endregion
