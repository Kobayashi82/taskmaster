/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Keys.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 11:35:45 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/30 21:14:37 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Parser.hpp"

#pragma endregion

#pragma region "Remove Comments"

	std::string ConfigParser::key_remove_comments(const std::string& line) const {
		char	quoteChar = 0;
		bool	escaped = false;

		for (size_t i = 0; i < line.length(); ++i) {
			char c = line[i];

			if (escaped)								{ escaped = false;			continue; }
			if (c == '\\')								{ escaped = true;			continue; }
			if (!quoteChar && (c == '"' || c == '\''))	{ quoteChar = c;			continue; }
			if (quoteChar && c == quoteChar)			{ quoteChar = 0;			continue; }

			if (!quoteChar && (c == '#' || c == ';'))	return (line.substr(0, i));
		}

		return (line);
	}

#pragma endregion

#pragma region "Valid"

	bool ConfigParser::key_valid(const std::string& section, const std::string& key) const {
		std::string sectionType = section_type(section);
		if (sectionType.empty()) return (false);

		auto it = validKeys.find(sectionType);
		if (it == validKeys.end()) return (false);

		return (it->second.count(toLower(key)));
	}

#pragma endregion

#pragma region "Parse"

	void ConfigParser::key_parse(const std::string& line) {
		if (trim(line).empty())									return;
		if (currentSection.empty() && !trim(line).empty())		throw std::runtime_error("Key found outside of a section: " + line);

		size_t pos = line.find('=');
		if (pos == std::string::npos)							throw std::runtime_error("[" + currentSection + "] " + line + ": invalid key");

		std::string key   = trim(toLower(line.substr(0, pos)));
		std::string value = trim(line.substr(pos + 1));

		if (key.empty())										throw std::runtime_error("[" + currentSection + "] Empty key");
		if (!key_valid(currentSection, key))					throw std::runtime_error("[" + currentSection + "] " + key + ": invalid key");

		if (value.empty())										throw std::runtime_error("[" + currentSection + "] " + key + ": empty value");

		std::string expanded_value;
		std::map<std::string, std::string> temp = environment;
		environment_add(temp, environment_config, true);
		expanded_value = environment_expand(temp, value, key == "environment");

		if (expanded_value.empty())								throw std::runtime_error("[" + currentSection + "] " + key + ": empty value");

		validate(currentSection, key, expanded_value);
		sections[currentSection][key] = expanded_value;
	}

#pragma endregion
