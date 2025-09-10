/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Group.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/02 17:23:05 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/10 18:31:18 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Taskmaster/Taskmaster.hpp"

	#include <unistd.h>															// gethostname()
	#include <filesystem>														// std::filesystem::path()

#pragma endregion

#pragma region "Constructors"

	Group::Group(const std::string _section) : section(_section), name(_section.substr(6)) {
		initialize();
	}

#pragma endregion

#pragma region "Configuration"

	#pragma region "Expand Vars"

		std::string Group::expand_vars(std::map<std::string, std::string>& env, const std::string& key) {
			if (key.empty()) return {};

			ConfigParser::ConfigEntry *entry = Config.get_value_entry(section, key);
			if (!entry) return {};

			try {
				entry->value = Utils::environment_expand(env, entry->value);
				entry->value = Utils::remove_quotes(entry->value);
				if (entry->value.empty()) {
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": empty value", ERROR, entry->line, entry->order);
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section][key], WARNING, entry->line, entry->order + 1);
					entry->value = Config.defaultValues[section][key];
				}
			}
			catch(const std::exception& e) {
				Utils::error_add(entry->filename, "[" + section + "] " + key + ": unclosed quote or unfinished escape sequence", ERROR, entry->line, entry->order);
				Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section.substr(0, 6)][key], WARNING, entry->line, entry->order + 1);
				entry->value = Config.defaultValues[section.substr(0, 6)][key];
			}

			if (key == "priority" && !Utils::valid_number(entry->value, 0, 999)) {
				Utils::error_add(entry->filename, "[" + section + "] " + key + ": must be a value between 0 and 999", ERROR, entry->line, entry->order);
				Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section.substr(0, 6)][key], WARNING, entry->line, entry->order + 2);
				entry->value = Config.defaultValues[section.substr(0, 6)][key];
			}

			return (entry->value);
		}

	#pragma endregion

	#pragma region "Initialize"

		void Group::initialize() {
			std::string configFile;
			uint16_t	order = 0;

			ConfigParser::ConfigEntry *entry = Config.get_value_entry(section, "priority");
			if (entry) { configFile = entry->filename; order = entry->order; }

			std::map<std::string, std::string> env;
			Utils::environment_initialize(env);

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
				catch (const std::exception& e)	{ Utils::error_add(entry->filename, "[" + section + "] command: unclosed quote or unfinished escape sequence", ERROR, entry->line, entry->order); }
				if (entry->value.empty())		  Utils::error_add(entry->filename, "[" + section + "] programs: empty value", ERROR, entry->line, entry->order);
			}
			else								  Utils::error_add(configFile, "[" + section + "] programs: required", ERROR, 0, order);
		}

	#pragma endregion

#pragma endregion
