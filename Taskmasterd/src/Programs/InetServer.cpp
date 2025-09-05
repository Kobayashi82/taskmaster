/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   InetServer.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/05 19:24:17 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/05 20:34:33 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Utils/Utils.hpp"
	#include "Config/Config.hpp"
	#include "Programs/TaskManager.hpp"
	#include "Programs/InetServer.hpp"

	#include <cstring>															// strerror()
	#include <unistd.h>															// gethostname()
	#include <filesystem>														// std::filesistem::path()
	#include <sstream>															// std::stringstream
	#include <iostream>

#pragma endregion

#pragma region "Validate"

	std::string InetServer::validate(const std::string& key, ConfigParser::ConfigEntry *entry) {
		if (key.empty() || !entry) return {};

		if (key == "password" && !entry->value.empty() && !Utils::valid_password(entry->value)) {
			Utils::error_add(entry->filename, "[" + section + "] " + key + ": invalid SHA format", ERROR, entry->line, entry->order);
			entry->value = "";
		}

		return (entry->value);
	}

#pragma region "Expand Vars (cambiar)"

	std::string InetServer::expand_vars(std::map<std::string, std::string>& env, const std::string& key) {
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
			if (key != "username" && key != "password") {
				Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section][key], WARNING, 0, entry->order + 1);
				entry->value = Config.defaultValues[section][key];
			}
		}

		std::string final_value = validate(key, entry);
		entry->value = original_value;
		return (final_value);
	}

#pragma endregion

#pragma region "Initialize"

	void InetServer::initialize() {
		std::string configFile;
		uint16_t	order = 0;
		disabled = false;

		if (!Config.has_section("inet_http_server")) { disabled = true; return; }

		ConfigParser::ConfigEntry *entry = Config.get_value_entry(section, "username");
		if (entry) { configFile = entry->filename; order = entry->order + 1; }

		std::map<std::string, std::string> env;
		Utils::environment_clone(env, TaskMaster.environment);

		char hostname[255];
		Utils::environment_add(env, "HOST_NAME", (!gethostname(hostname, sizeof(hostname))) ? std::string(hostname) : "unknown");
		if (!configFile.empty()) Utils::environment_add(env, "HERE", std::filesystem::path(configFile).parent_path());

		username	= expand_vars(env, "username");
		password	= expand_vars(env, "password");

		entry = Config.get_value_entry(section, "port");
		if (entry) {
			try {
				entry->value = Utils::environment_expand(env, entry->value);
				entry->value = Utils::remove_quotes(entry->value);
			}
			catch (const std::exception& e) { Utils::error_add(entry->filename, "[" + section + "] port: unclosed quote or unfinished escape sequence", ERROR, entry->line, entry->order); }
		
			if (!entry->value.empty() && !Utils::valid_port(entry->value)) {
				Utils::error_add(entry->filename, "[" + section + "] port: must be a valid TCP host:port", ERROR, entry->line, entry->order);
				entry->value = ""; disabled = true;
			}

			if (entry->value.empty()) {
				Utils::error_add(entry->filename, "[" + section + "] port: empty value", ERROR, entry->line, entry->order);
				disabled = true;
			}
		} else {
			if (!username.empty() && !password.empty())
				Utils::error_add(configFile, "[" + section + "] port: required", ERROR, 0, order);
			disabled = true;
		}
	}

#pragma endregion
