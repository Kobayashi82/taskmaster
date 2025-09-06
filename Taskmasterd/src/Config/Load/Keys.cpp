/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Keys.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 11:35:45 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/06 20:01:08 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Utils/Utils.hpp"
	#include "Config/Config.hpp"

	#include <unistd.h>															// gethostname()
	#include <filesystem>														// std::filesystem::path()

#pragma endregion

#pragma region "Getters"

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

#pragma region "Valid"

	bool ConfigParser::key_valid(const std::string& section, const std::string& key) const {
		std::string sectionType = section_type(section);
		if (sectionType.empty())				return (false);

		auto it = validKeys.find(sectionType);
		if (it == validKeys.end())				return (false);

		return (it->second.count(Utils::toLower(key)));
	}

#pragma endregion

#pragma region "Parse"

	int ConfigParser::key_parse(const std::string& line, int line_number, std::string& filename) {
		std::string check_inv_chars = line;
		size_t open_sec = check_inv_chars.find_first_of("[");
		size_t equal_char = check_inv_chars.find_first_of("=");
		if (open_sec < equal_char) {
			Utils::error_add(filename, line + " invalid section", WARNING, line_number, order);
			currentSection = ""; return (2);
		}

		if (currentSection.empty())				{ Utils::error_add(filename, "key found outside of a section: " + line, ERROR, line_number, order);					return (1); }

		size_t pos = line.find('=');
		if (pos == std::string::npos)			{ Utils::error_add(filename, "[" + currentSection + "] " + line + ": invalid key", ERROR, line_number, order);		return (1); }

		std::string key   = Utils::trim(Utils::toLower(line.substr(0, pos)));
		std::string value = Utils::trim(line.substr(pos + 1));

		if (key.empty())						{ Utils::error_add(filename, "[" + currentSection + "] Empty key", ERROR, line_number, order);						return (1); }
		if (!key_valid(currentSection, key))	{ Utils::error_add(filename, "[" + currentSection + "] " + key + ": invalid key", ERROR, line_number, order);		return (1); }
		if (value.empty())						{ Utils::error_add(filename, "[" + currentSection + "] " + key + ": empty value", ERROR, line_number, order);		return (1); }

		if (key == "stdout_events_enabled")		{ Utils::error_add(filename, "[" + currentSection + "] " + key + ": not implemented", WARNING, line_number, order);	return (1); }
		if (key == "stdout_capture_maxbytes")	{ Utils::error_add(filename, "[" + currentSection + "] " + key + ": not implemented", WARNING, line_number, order);	return (1); }
		if (key == "stderr_events_enabled")		{ Utils::error_add(filename, "[" + currentSection + "] " + key + ": not implemented", WARNING, line_number, order);	return (1); }
		if (key == "stderr_capture_maxbytes")	{ Utils::error_add(filename, "[" + currentSection + "] " + key + ": not implemented", WARNING, line_number, order);	return (1); }

		ConfigEntry entry;
		if (currentSection == "include" && key == "files") {
			char hostname[255];
			std::map<std::string, std::string> env;
			Utils::environment_initialize(env);
			Utils::environment_add(env, "HOST_NAME", (!gethostname(hostname, sizeof(hostname))) ? std::string(hostname) : "unknown");
			Utils::environment_add(env, "HERE", Utils::expand_path(std::filesystem::path(filename).parent_path(), "", true, false));

			try { value = Utils::environment_expand(env, value);
			} catch (const std::exception& e) {
				Utils::error_add(entry.filename, "[" + currentSection + "] " + key + ": unclosed quote or unfinished escape sequence", ERROR, entry.line, entry.order);
				value = "";
			}
			env.clear();
		}
		entry.value = value;
		entry.filename = filename;
		entry.line = line_number;
		entry.order = order;
		sections[currentSection][key] = entry;

		return (0);
	}

#pragma endregion
