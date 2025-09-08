/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Section.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 11:34:51 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/08 17:18:22 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Utils/Utils.hpp"
	#include "Config/Config.hpp"

#pragma endregion

#pragma region "Has Section"

	bool ConfigParser::has_section(const std::string& section) const {
		return (sections.find(section) != sections.end());
	}

#pragma endregion

#pragma region "Is Section"

	bool ConfigParser::is_section(const std::string& line) const {
		std::string trimmed = Utils::trim(line);
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

		return {};
	}

#pragma endregion

#pragma region "Extract"

	std::string ConfigParser::section_extract(const std::string& line) const {
		std::string trimmed = Utils::trim(line);
		return (Utils::trim(trimmed.substr(1, trimmed.length() - 2)));
	}

#pragma endregion

#pragma region "Parse"

	int ConfigParser::section_parse(const std::string& line, int line_number, std::string& filename) {
		std::string section		= section_extract(line);
		std::string sectionType	= section_type(section);

		if (in_reloading) {
			if (sectionType.empty())															{ currentSection = "";	Utils::error_add(filename, "[" + section + "] unkown section", WARNING, line_number, order);								return (1); }
			if (section == "program:")															{ currentSection = "";	Utils::error_add(filename, "[" + section + "] program name is missing", ERROR, line_number, order);							return (1); }
			if (section == "group:")															{ currentSection = "";	Utils::error_add(filename, "[" + section + "] group name is missing", ERROR, line_number, order);							return (1); }
			if (section == "include" && sections.find(section) != sections.end())				{ currentSection = "";																																return (1); }
			if (section != "include" && sectionType != "program:" && sectionType != "group:")	{ currentSection = "";																																return (1); }
		} else {
			if (section == "include" && sections.find(section) != sections.end())				{ currentSection = "";	Utils::error_add(filename, "[" + section + "] invalid section", WARNING, line_number, order);								return (1); }
			if (section == "program:")															{ currentSection = "";	Utils::error_add(filename, "[" + section + "] program name is missing", ERROR, line_number, order);							return (1); }
			if (section == "group:")															{ currentSection = "";	Utils::error_add(filename, "[" + section + "] group name is missing", ERROR, line_number, order);							return (1); }
			if (section.substr(0, 13) == "fcgi-program:")										{ currentSection = "";	Utils::error_add(filename, "[" + section + "] not implemented", WARNING, line_number, order);								return (1); }
			if (section.substr(0, 14) == "eventlistener:")										{ currentSection = "";	Utils::error_add(filename, "[" + section + "] not implemented", WARNING, line_number, order);								return (1); }
			if (section.substr(0, 13) == "rpcinterface:")										{ currentSection = "";	Utils::error_add(filename, "[" + section + "] not implemented", WARNING, line_number, order);								return (1); }
			if (section == "taskmasterctl") 													{ currentSection = "";																																return (1); }
			if (sectionType.empty())															{ currentSection = "";	Utils::error_add(filename, "[" + section + "] unkown section", WARNING, line_number, order);								return (1); }
		}

		std::string check_inv_chars = section;
		if (sectionType == "program:")	check_inv_chars = section.substr(8);
		if (sectionType == "group:")	check_inv_chars = section.substr(6);
		if (check_inv_chars.find_first_of(":[]") != std::string::npos)							{ currentSection = "";	Utils::error_add(filename, "[" + section + "] contains invalid characters (':' or brackets)", ERROR, line_number, order);	return (1); }

		currentSection = section;
		
		if (!sectionType.empty()) {
			auto defaultSectionIt = defaultValues.find(sectionType);
			if (defaultSectionIt != defaultValues.end()) {
				for (const auto& kv : defaultSectionIt->second) {
					ConfigEntry entry;
					entry.value = kv.second;
					entry.filename = filename;
					entry.line = line_number;
					entry.order = order++;
					sections[currentSection][kv.first] = entry;
				}
			}
		} else sections[currentSection] = std::map<std::string, ConfigEntry>();

		return (0);
	}

#pragma endregion
