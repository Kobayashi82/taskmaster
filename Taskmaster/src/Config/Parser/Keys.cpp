/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Keys.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 11:35:45 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/02 14:56:46 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Config.hpp"
	#include "Logging/TaskmasterLog.hpp"

	#include <unistd.h>															// gethostname()

#pragma endregion

#pragma region "Remove Comments"

	std::string ConfigParser::remove_comments(const std::string& line) const {
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

	int ConfigParser::key_parse(const std::string& line, int line_number, std::string& filename) {
		if (currentSection.empty())				{ error_add(filename, "key found outside of a section: " + line, ERROR, line_number, order); return (1); }

		size_t pos = line.find('=');
		if (pos == std::string::npos)			{ error_add(filename, "[" + currentSection + "] " + line + ": invalid key", ERROR, line_number, order); return (1); }

		std::string key   = trim(toLower(line.substr(0, pos)));
		std::string value = trim(line.substr(pos + 1));

		if (key.empty())						{ error_add(filename, "[" + currentSection + "] Empty key", ERROR, line_number, order); return (1); }
		if (!key_valid(currentSection, key))	{ error_add(filename, "[" + currentSection + "] " + key + ": invalid key", ERROR, line_number, order); return (1); }
		if (value.empty())						{ error_add(filename, "[" + currentSection + "] " + key + ": empty value", ERROR, line_number, order); return (1); }

		ConfigEntry entry;
		if (currentSection == "include" && key == "files") {
			char hostname[255];
			environment_initialize(environment);
			environment_add(environment, "HOST_NAME", (!gethostname(hostname, sizeof(hostname))) ? std::string(hostname) : "unknown");
			environment_add(environment, "HERE", expand_path(".", "", true, false));

			try { value = environment_expand(environment, value, ", \t\n");
			} catch (const std::exception& e) {
				error_add(entry.filename, "[" + currentSection + "] " + key + ": unclosed quote or unfinished escape sequence", ERROR, entry.line, entry.order);
				value = "";
			}
			environment.clear();
		}
		entry.value = value;
		entry.filename = filename;
		entry.line = line_number;
		entry.order = order;
		sections[currentSection][key] = entry;

		return (0);
	}

#pragma endregion
