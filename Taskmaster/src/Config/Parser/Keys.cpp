/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Keys.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 11:35:45 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/27 12:44:53 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Parser.hpp"

	#include <stdexcept>														// std::runtime_error

#pragma endregion

#pragma region "Key=Value"

	#pragma region "Is Valid"

		bool ConfigParser::valid_key(const std::string& section, const std::string& key) const {
			std::string sectionType = section_type(section);
			if (sectionType.empty()) return (false);

			auto it = validKeys.find(sectionType);
			if (it == validKeys.end()) return (false);

			return (it->second.count(toLower(key)) > 0);
		}

	#pragma endregion

	#pragma region "Parse"

		void ConfigParser::parse_key(const std::string& line) {
			if (currentSection.empty() && !trim(line).empty()) throw std::runtime_error("Key found outside of section: " + line);
			if (trim(line).empty()) return;

			size_t pos = line.find('=');
			if (pos == std::string::npos) throw std::runtime_error("Invalid key '" + line + "' in section [" + currentSection + "]");

			std::string key = trim(line.substr(0, pos));
			std::string value = trim(line.substr(pos + 1));

			if (key.empty()) throw std::runtime_error("Empty key in section [" + currentSection + "]");
			if (!valid_key(currentSection, key)) throw std::runtime_error("Invalid key '" + key + "' in section [" + currentSection + "]");

			key = toLower(key);
			if (value.empty()) throw std::runtime_error("Empty value for '" + key + "' in section [" + currentSection + "]");
			validate(currentSection, key, value);
			sections[currentSection][key] = value;
		}

	#pragma endregion

#pragma endregion
