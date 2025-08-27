/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Section.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 11:34:51 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/27 12:44:24 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Parser.hpp"

	#include <stdexcept>														// std::runtime_error

#pragma endregion

#pragma region "Section"

	#pragma region "Type"

		std::string ConfigParser::section_type(const std::string& section) const {
			for (const auto& validSection : validSections) {
				if (validSection.back() == ':') {
					if (section.substr(0, validSection.length()) == validSection)	return (validSection);
				} else if (section == validSection)									return (validSection);
			}

			return ("");
		}

	#pragma endregion

	#pragma region "Is Valid"

		bool ConfigParser::valid_section(const std::string& section) const {
			if (validSections.count(section)) return (true);

			for (const auto& validSection : validSections) {
				if (validSection.back() == ':' &&
					section.substr(0, validSection.length()) == validSection &&
					section.length() > validSection.length()) {
					return (true);
				}
			}

			return (false);
		}

	#pragma endregion

	#pragma region "Is Section"

		bool ConfigParser::is_section(const std::string& line) const {
			std::string trimmed = trim(line);
			return trimmed.size() >= 2 && trimmed[0] == '[' && trimmed.back() == ']';
		}

	#pragma endregion

	#pragma region "Extract"

		std::string ConfigParser::extract_section(const std::string& line) const {
			std::string trimmed = trim(line);
			return (trimmed.substr(1, trimmed.length() - 2));
		}

	#pragma endregion

	#pragma region "Parse"

		void ConfigParser::parse_section(const std::string& line) {
			std::string section = extract_section(line);

			if (!valid_section(section))					{ currentSection = ""; throw std::runtime_error("Invalid section:\t[" + section + "]"); }
			if (sections.find(section) != sections.end())	{ currentSection = ""; throw std::runtime_error("Duplicate section:\t[" + section + "]"); }

			currentSection = section;
			if (currentSection == "taskmasterctl") { throw std::runtime_error("Ignore section"); }

			std::string sectionType = section_type(section);
			if (!sectionType.empty()) {
				auto defaultSectionIt = defaultValues.find(sectionType);
				if (defaultSectionIt != defaultValues.end()) sections[currentSection] = defaultSectionIt->second;
			} else {
				sections[currentSection] = std::map<std::string, std::string>();
			}

			if (currentSection == "include") in_include = true;
		}

	#pragma endregion

#pragma endregion
