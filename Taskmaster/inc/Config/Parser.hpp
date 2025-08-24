/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/24 21:47:27 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/24 22:03:49 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include <string>															// std::string
	#include <set>																// std::set
	#include <map>																// std::map
	#include <vector>															// std::vector

#pragma endregion

#pragma region "Parser"

	class Parser {

		private:

			// Variables
			std::string													currentSection;
			std::map<std::string, std::map<std::string, std::string>>	sections;
			std::set<std::string>										validSections;
			std::map<std::string, std::set<std::string>>				validKeys;
			std::map<std::string, std::map<std::string, std::string>>	defaultValues;

			// Initialize
			void	initialize();
			void	default_values();

			// Utils
			std::string	trim(const std::string& str) const;
			std::string	toLower(const std::string& str) const;

			// Key=Value
			bool isValidKey(const std::string& section, const std::string& key) const;
			void parseKeyValue(const std::string& line);

			// Section
			std::string SectionType(const std::string& section) const;
			bool isValidSection(const std::string& section) const;
			std::string extractSection(const std::string& line) const;
			void parseSection(const std::string& line);

			// Line
			bool isSection(const std::string& line) const;
			bool isComment(const std::string& line) const;
			void parseLine(const std::string& line);

		public:

			// Constructors
			Parser();
			Parser(const Parser&) = delete;
			~Parser() = default;

			// Overloads
			Parser& operator=(const Parser&) = delete;

			// File
			int parseFile(const std::string& filePath);

			// Getters
			bool								hasSection(const std::string& section) const;
			std::string							getValue(const std::string& section, const std::string& key, const std::string& defaultValue = "") const;
			std::map<std::string, std::string>	getSection(const std::string& section) const;
			std::map<std::string, std::string>	getSectionWithDefaults(const std::string& section) const;
			std::vector<std::string>			getProgramSections() const;
			std::vector<std::string>			getGroupSections() const;

			// Debug
			void	clear();
			void	printConfig() const;
	};

#pragma endregion
