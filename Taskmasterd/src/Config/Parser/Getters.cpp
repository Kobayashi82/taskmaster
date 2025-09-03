/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Getters.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 11:37:28 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/04 00:02:33 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Utils/Utils.hpp"
	#include "Config/Config.hpp"

	#include <iostream>															// std::cout()
	#include <algorithm>														// std::replace()

#pragma endregion

#pragma region "Value"

	ConfigParser::ConfigEntry* ConfigParser::get_value_entry(const std::string& section, const std::string& key) {
		auto sectionIt = sections.find(section);

		if (sectionIt != sections.end()) {
			auto keyIt = sectionIt->second.find(Utils::toLower(key));
			if (keyIt != sectionIt->second.end()) return (&keyIt->second);
		}

		return {};
	}

	std::string ConfigParser::get_value(const std::string& section, const std::string& key) const {
		auto sectionIt = sections.find(section);

		if (sectionIt != sections.end()) {
			auto keyIt = sectionIt->second.find(Utils::toLower(key));
			if (keyIt != sectionIt->second.end()) return (keyIt->second.value);
		}

		return {};
	}

#pragma endregion

#pragma region "Has Section"

	bool ConfigParser::has_section(const std::string& section) const {
		return (sections.find(section) != sections.end());
	}

#pragma endregion

#pragma region "Print"

	void ConfigParser::print() const {
		for (const auto& section : sections) {
			std::cout << "[" << section.first << "]" << std::endl;
			for (const auto& kv : section.second) {
				std::string value = kv.second.value;
				std::replace(value.begin(), value.end(), '\n', ',');
				std::cout << kv.first << " = " << value << std::endl;
			}
			std::cout << std::endl;
		}
	}

#pragma endregion
