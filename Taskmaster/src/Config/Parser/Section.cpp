/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Section.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 11:34:51 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/01 13:01:41 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Parser.hpp"
	#include "Logging/TaskmasterLog.hpp"

#pragma endregion

#pragma region "Is Section"

	bool ConfigParser::is_section(const std::string& line) const {
		std::string trimmed = trim(line);
		return (trimmed.size() >= 2 && trimmed[0] == '[' && trimmed.back() == ']');
	}

#pragma endregion

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

#pragma region "Extract"

	std::string ConfigParser::section_extract(const std::string& line) const {
		std::string trimmed = trim(line);
		return (trimmed.substr(1, trimmed.length() - 2));
	}

#pragma endregion

#pragma region "Parse"

	int ConfigParser::section_parse(const std::string& line, int line_number, std::string& filename) {
		std::string section = section_extract(line);

		if (section_type(section).empty())				{ currentSection = ""; error_add(filename, "[" + section + "] invalid section", WARNING, line_number, order); return (1); }
		if (section == "taskmasterctl")					{ currentSection = ""; return (1); }

		currentSection = section;

		std::string sectionType = section_type(section);
		if (!sectionType.empty()) {
			auto defaultSectionIt = defaultValues.find(sectionType);
			if (defaultSectionIt != defaultValues.end()) {
				for (const auto& kv : defaultSectionIt->second) {
					ConfigEntry entry;
					entry.value = kv.second;
					entry.filename = filename;
					entry.line = line_number;
					entry.order = order;
					sections[currentSection][kv.first] = entry;
				}
			}
		} else sections[currentSection] = std::map<std::string, ConfigEntry>();

		return (0);
	}

#pragma endregion
