/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Keys.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 11:35:45 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/10 18:35:48 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

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

	int ConfigParser::key_parse(const std::string& line, int line_number, std::string& filename, bool start_space) {
		bool ignore = (start_space && in_environment);

		std::string check_inv_chars = line;
		size_t open_sec = check_inv_chars.find_first_of("[");
		size_t equal_char = check_inv_chars.find_first_of("=");
		if (!ignore && open_sec < equal_char)			{ Utils::error_add(filename, line + " invalid section", WARNING, line_number, order);	currentSection = "";		return (2); }

		if (currentSection.empty())						{ Utils::error_add(filename, "key found outside of a section: " + line, ERROR, line_number, order);					return (1); }

		size_t pos = line.find('=');
		if (!ignore && pos == std::string::npos)		{ Utils::error_add(filename, "[" + currentSection + "] " + line + ": invalid key", ERROR, line_number, order);		return (1); }

		std::string key   = Utils::trim(Utils::toLower(line.substr(0, pos)));
		std::string value = Utils::trim(line.substr(pos + 1));

		if (!ignore && key.empty())						{ Utils::error_add(filename, "[" + currentSection + "] Empty key", ERROR, line_number, order);						return (1); }
		if (!ignore && !key_valid(currentSection, key))	{ Utils::error_add(filename, "[" + currentSection + "] " + key + ": invalid key", ERROR, line_number, order);		return (1); }
		if (key == "stdout_events_enabled")				{ Utils::error_add(filename, "[" + currentSection + "] " + key + ": not implemented", WARNING, line_number, order);	return (1); }
		if (key == "stdout_capture_maxbytes")			{ Utils::error_add(filename, "[" + currentSection + "] " + key + ": not implemented", WARNING, line_number, order);	return (1); }
		if (key == "stderr_events_enabled")				{ Utils::error_add(filename, "[" + currentSection + "] " + key + ": not implemented", WARNING, line_number, order);	return (1); }
		if (key == "stderr_capture_maxbytes")			{ Utils::error_add(filename, "[" + currentSection + "] " + key + ": not implemented", WARNING, line_number, order);	return (1); }
		if (key == "environment" && value.empty())		{ Utils::error_add(filename, "[" + currentSection + "] " + key + ": empty value", ERROR, line_number, order);		return (1); }

		if (currentSection == "include" && key == "files") {
			if (value.empty())							{ Utils::error_add(filename, "[" + currentSection + "] " + key + ": empty value", ERROR, line_number, order);		return (1); }

			char hostname[255];
			std::map<std::string, std::string> env;
			Utils::environment_initialize(env);
			Utils::environment_add(env, "HOST_NAME", (!gethostname(hostname, sizeof(hostname))) ? std::string(hostname) : "unknown");
			Utils::environment_add(env, "HERE", Utils::expand_path(std::filesystem::path(filename).parent_path(), "", true, false));

			try { value = Utils::environment_expand(env, value); }
			catch (const std::exception& e) { Utils::error_add(filename, "[" + currentSection + "] " + key + ": unclosed quote or unfinished escape sequence", ERROR, line_number, order); value = ""; }
		}
		else if (start_space && in_environment) {
			ConfigEntry *e_entry = get_value_entry(currentSection, "environment");
			if (e_entry) {
				if (value.back() == '\n' || value.back() == ',') value.pop_back();
				if (e_entry->value.back() == '\n' || e_entry->value.back() == ',') e_entry->value.pop_back();
				e_entry->value += ", " + line;
			}
			return (0);
		}
		else if (key == "environment" && (currentSection == "taskmasterd" || currentSection.substr(0, 8) == "program:"))	in_environment = true;
		else																												in_environment = false;

		ConfigEntry entry;
		entry.value = value;
		entry.filename = filename;
		entry.line = line_number;
		entry.order = order;
		sections[currentSection][key] = entry;

		return (0);
	}

#pragma endregion
