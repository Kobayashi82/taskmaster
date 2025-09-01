/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Getters.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 11:37:28 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/01 12:20:11 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Parser.hpp"

#pragma endregion

#pragma region "Value"

	std::string ConfigParser::get_value(const std::string& section, const std::string& key, const std::string& defaultValue) const {
		(void) defaultValue;

		auto sectionIt = sections.find(section);
		std::string normalizedKey = toLower(key);

		// Search in config loaded
		if (sectionIt != sections.end()) {
			auto keyIt = sectionIt->second.find(normalizedKey);
			if (keyIt != sectionIt->second.end()) return (keyIt->second.value);
		}

		// // Search in default config
		// std::string sectionType = section_type(section);
		// if (!sectionType.empty()) {
		// 	auto defaultSectionIt = defaultValues.find(sectionType);
		// 	if (defaultSectionIt != defaultValues.end()) {
		// 		auto defaultKeyIt = defaultSectionIt->second.find(normalizedKey);
		// 		if (defaultKeyIt != defaultSectionIt->second.end()) return (defaultKeyIt->second);
		// 	}
		// }

		// Fallback to defaultValue
		// return (defaultValue);

		return ("");
	}

#pragma endregion

#pragma region "Section"

	#pragma region "Has Section"

		bool ConfigParser::has_section(const std::string& section) const {
			return (sections.find(section) != sections.end());
		}

	#pragma endregion

	#pragma region "Section"

		std::map<std::string, ConfigParser::ConfigEntry> ConfigParser::get_section(const std::string& section, bool use_defaults) const {
			if (!use_defaults) {
				auto it = sections.find(section);
				if (it != sections.end()) return (it->second);
				
				return (std::map<std::string, ConfigParser::ConfigEntry>());
			}

			std::map<std::string, ConfigParser::ConfigEntry> result;

			// Defaults
			std::string sectionType = section_type(section);
			if (!sectionType.empty()) {
				auto defaultSectionIt = defaultValues.find(sectionType);
				if (defaultSectionIt != defaultValues.end()) {
					for (const auto& kv : defaultSectionIt->second) {
						ConfigEntry entry;
						entry.value = kv.second;
						entry.filename = "defaults";
						entry.line = 0;
						entry.order = 0;
						result[kv.first] = entry;
					}
				}
			}

			auto sectionIt = sections.find(section);
			if (sectionIt != sections.end()) {
				for (const auto& kv : sectionIt->second) result[kv.first] = kv.second;
			}

			return (result);
		}

	#pragma endregion

#pragma endregion

#pragma region "Program"

	std::vector<std::string> ConfigParser::get_program() const {
		std::vector<std::string> programs;
		for (const auto& section : sections) {
			if (section.first.substr(0, 8) == "program:") programs.push_back(section.first);
		}

		return (programs);
	}

#pragma endregion

#pragma region "Group"

	std::vector<std::string> ConfigParser::get_group() const {
		std::vector<std::string> groups;
		for (const auto& section : sections) {
			if (section.first.substr(0, 6) == "group:") groups.push_back(section.first);
		}

		return (groups);
	}

#pragma endregion
