/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Section.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 11:34:51 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/29 21:00:20 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Parser.hpp"

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

	void ConfigParser::section_parse(const std::string& line) {
		std::string section = section_extract(line);

		if (section_type(section).empty())				{ currentSection = ""; throw std::runtime_error("Invalid section:\t[" + section + "]"); }
		if (sections.find(section) != sections.end())	{ currentSection = ""; throw std::runtime_error("Duplicate section:\t[" + section + "]"); }
		if (section == "taskmasterctl")					{ currentSection = ""; throw std::runtime_error("Ignore section"); }

		currentSection = section;

		std::string sectionType = section_type(section);
		if (!sectionType.empty()) {
			auto defaultSectionIt = defaultValues.find(sectionType);
			if (defaultSectionIt != defaultValues.end()) sections[currentSection] = defaultSectionIt->second;
		} else {
			sections[currentSection] = std::map<std::string, std::string>();
		}

		if (currentSection.substr(0, 8) == "program:") {
			std::string program_name = trim(currentSection.substr(8));
			if (!program_name.empty()) environment_add(environment_config, "PROGRAM_NAME", program_name);
		}
		else if (currentSection.substr(0, 6) == "group:") {
			std::string group_name = trim(currentSection.substr(7));
			if (!group_name.empty()) environment_add(environment_config, "GROUP_NAME", group_name);
		} else {
			environment_del(environment_config, "PROGRAM_NAME");
			environment_del(environment_config, "GROUP_NAME");
		}
	}

#pragma endregion
