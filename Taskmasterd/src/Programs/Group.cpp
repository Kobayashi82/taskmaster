/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Group.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/02 17:23:05 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/06 17:42:13 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Utils/Utils.hpp"
	#include "Config/Config.hpp"
	#include "Programs/TaskManager.hpp"
	#include "Programs/Group.hpp"

	#include <cstring>															// strerror()
	#include <unistd.h>															// gethostname()
	#include <filesystem>														// std::filesistem::path()
	#include <sstream>															// std::stringstream
	#include <iostream>

#pragma endregion

#pragma region "Constructors"

	Group::Group(const std::string _section) : section(_section), name(_section.substr(6)) {
		initialize();
	}

#pragma endregion

#pragma region "Validate"

	std::string Group::validate(const std::string& key, ConfigParser::ConfigEntry *entry) {
		if (key.empty() || !entry) return {};

		if (key == "priority" && !Utils::valid_number(entry->value, 0, 999)) {
			Utils::error_add(entry->filename, "[" + section + "] " + key + ": must be a value between 0 and 999", ERROR, entry->line, entry->order);
			Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section][key], WARNING, 0, entry->order + 1);
			entry->value = Config.defaultValues[section][key];
		}

		return (entry->value);
	}

#pragma region "Expand Vars (cambiar)"

	std::string Group::expand_vars(std::map<std::string, std::string>& env, const std::string& key) {
		if (key.empty()) return {};

		ConfigParser::ConfigEntry *entry = Config.get_value_entry(section, key);
		if (!entry) return {};

		std::string original_value = entry->value;

		try {
			entry->value = Utils::environment_expand(env, entry->value);
			entry->value = Utils::remove_quotes(entry->value);
		}
		catch(const std::exception& e) {
			Utils::error_add(entry->filename, "[" + section + "] " + key + ": unclosed quote or unfinished escape sequence", ERROR, entry->line, entry->order);
			Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section][key], WARNING, 0, entry->order + 1);
			entry->value = Config.defaultValues[section][key];
		}

		std::string final_value = validate(key, entry);
		entry->value = original_value;
		return (final_value);
	}

#pragma endregion

#pragma region "Initialize"

	void Group::initialize() {
		std::string configFile;
		uint16_t	order = 0;
		disabled = false;

		ConfigParser::ConfigEntry *entry = Config.get_value_entry(section, "priority");
		if (entry) { configFile = entry->filename; order = entry->order + 1; }

		std::map<std::string, std::string> env;
		Utils::environment_clone(env, TaskMaster.environment);

		char hostname[255];
		Utils::environment_add(env, "HOST_NAME", (!gethostname(hostname, sizeof(hostname))) ? std::string(hostname) : "unknown");
		if (!configFile.empty()) Utils::environment_add(env, "HERE", std::filesystem::path(configFile).parent_path());
		Utils::environment_add(env, "GROUP_NAME", name);

		priority = Utils::parse_number(expand_vars(env, "priority"), 0, 999, 999);

		entry = Config.get_value_entry(section, "programs");
		if (entry) {
			try {
				entry->value = Utils::environment_expand(env, entry->value);
				entry->value = Utils::remove_quotes(entry->value);
			}
			catch (const std::exception& e) { Utils::error_add(entry->filename, "[" + section + "] command: unclosed quote or unfinished escape sequence", ERROR, entry->line, entry->order); }

			if (entry->value.empty()) {
				Utils::error_add(entry->filename, "[" + section + "] programs: empty value", ERROR, entry->line, entry->order);
				disabled = true;
			}
		} else {
			Utils::error_add(configFile, "[" + section + "] programs: required", ERROR, 0, order);
			disabled = true;
		}

		if (!disabled) {
			std::set<std::string> program_names;
			std::stringstream ss(entry->value); std::string token;
			while (std::getline(ss, token, ',')) program_names.insert(Utils::trim(token));

			for (auto& program : TaskMaster.programs) {
				if (program_names.find(program.name) != program_names.end()) {
					if (!program.disabled) {
						programs.push_back(program.name);
						program.groups.push_back(name);
					}
				}
			}
		}
	}

#pragma endregion
