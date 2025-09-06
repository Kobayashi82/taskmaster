/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UnixServer.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/05 19:24:11 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/06 11:11:20 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Utils/Utils.hpp"
	#include "Config/Config.hpp"
	#include "Programs/TaskManager.hpp"
	#include "Programs/UnixServer.hpp"
	#include "Logging/TaskmasterLog.hpp"

	#include <cstring>															// strerror()
	#include <unistd.h>															// gethostname(), close()
	#include <filesystem>														// std::filesistem::path()
	#include <sstream>															// std::stringstream
	#include <sys/socket.h>														// socket()
	// #include <netinet/in.h>														// struct sockaddr
	#include <sys/un.h>															// struct sockaddr_un
	#include <iostream>

#pragma endregion

#pragma region "Initialize"

	#pragma region "Validate"

		std::string UnixServer::validate(const std::string& key, ConfigParser::ConfigEntry *entry) {
			if (key.empty() || !entry) return {};

			if (key == "chmod" && !Utils::valid_umask(entry->value)) {
				Utils::error_add(entry->filename, "[" + section + "] " + key + ": must be in octal format", ERROR, entry->line, entry->order);
				Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section][key], WARNING, 0, entry->order + 1);
				entry->value = Config.defaultValues[section][key];
			}

			if (key == "chown" && !Utils::valid_chown(entry->value)) {
				Utils::error_add(entry->filename, "[" + section + "] " + key + ": invalid user or group", ERROR, entry->line, entry->order);
				entry->value = "";
			}

			if (key == "password" && !entry->value.empty() && !Utils::valid_password(entry->value)) {
				Utils::error_add(entry->filename, "[" + section + "] " + key + ": invalid SHA format", ERROR, entry->line, entry->order);
				entry->value = "";
			}

			return (entry->value);
		}

	#pragma endregion

	#pragma region "Expand Vars (cambiar)"

		std::string UnixServer::expand_vars(std::map<std::string, std::string>& env, const std::string& key) {
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

		void UnixServer::initialize() {
			std::string configFile;
			uint16_t	order = 0;
			disabled = false;
			sockfd = 0;

			section = "unix_http_server";
			if (!Config.has_section("unix_http_server")) { disabled = true; return; }

			ConfigParser::ConfigEntry *entry = Config.get_value_entry(section, "username");
			if (entry) { configFile = entry->filename; order = entry->order + 1; }

			std::map<std::string, std::string> env;
			Utils::environment_clone(env, TaskMaster.environment);

			char hostname[255];
			Utils::environment_add(env, "HOST_NAME", (!gethostname(hostname, sizeof(hostname))) ? std::string(hostname) : "unknown");
			if (!configFile.empty()) Utils::environment_add(env, "HERE", std::filesystem::path(configFile).parent_path());

			std::string chmod_str = expand_vars(env, "chmod");
			chmod = static_cast<uint16_t>(std::stoi(chmod_str, nullptr, 8));

			std::string chown_str = expand_vars(env, "chown");
			if (chown_str.empty()) disabled = true;
			else {
				// separar chown_str en chown_user y chown_group
			}
			username = expand_vars(env, "username");
			password = expand_vars(env, "password");

			entry = Config.get_value_entry(section, "file");
			if (entry) {
				try {
					entry->value = Utils::environment_expand(env, entry->value);
					entry->value = Utils::remove_quotes(entry->value);
				}
				catch (const std::exception& e) { Utils::error_add(entry->filename, "[" + section + "] file: unclosed quote or unfinished escape sequence", ERROR, entry->line, entry->order); }

				std::string dir = TaskMaster.directory;
				if (!Utils::valid_path(entry->value, dir)) {
					if (entry->value != Config.defaultValues[section]["file"]) {
						Utils::error_add(entry->filename, "[" + section + "] file: invalid path - " + std::string(strerror(errno)), ERROR, entry->line, entry->order);
						Utils::error_add(entry->filename, "[" + section + "] file: reset to default value: " + Config.defaultValues[section]["file"], WARNING, 0, entry->order + 1);
						entry->value = Config.defaultValues[section]["file"];
						if (!Utils::valid_path(entry->value, dir)) {
							Utils::error_add(entry->filename, "[" + section + "] file: failed to use default value - " + std::string(strerror(errno)), ERROR, entry->line, entry->order);
							entry->value = "NONE"; disabled = true;
						}
					} else {
						Utils::error_add(entry->filename, "[" + section + "] file: invalid path - " + std::string(strerror(errno)), ERROR, entry->line, entry->order);
						entry->value = "NONE"; disabled = true;
					}
				}
				if (Utils::toUpper(entry->value) != "NONE") entry->value = Utils::expand_path(entry->value, dir);

				if (entry->value.empty()) {
					Utils::error_add(entry->filename, "[" + section + "] file: empty value", ERROR, entry->line, entry->order);
					disabled = true;
				}
				if (!disabled) file = entry->value;
			} else {
				if (!username.empty() && !password.empty())
					Utils::error_add(configFile, "[" + section + "] file: required", ERROR, 0, order);
				disabled = true;
			}
		}

	#pragma endregion

#pragma endregion

#pragma region "Start"

	int UnixServer::start() {
		sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
		if (sockfd == -1) {
			Log.error("Unix Server: failed to create socket - " + std::string(strerror(errno)));
			disabled = true; return (1);
    	}
		Log.debug("Unix Server: socket created successfully");

		sockaddr_un addr;
		std::memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_UNIX;
		std::strcpy(addr.sun_path, file.c_str());

		unlink(addr.sun_path);

		if (bind(sockfd, (sockaddr *)&addr, sizeof(addr)) == -1) {
			Log.error("Unix Server: failed to bind socket - " + std::string(strerror(errno)));
			::close(sockfd); disabled = true; return (1);
		}
		Log.debug("Unix Server: socket bound to path " + file);

		if (listen(sockfd, 50) == -1) {
			Log.error("Unix Server: failed to listen on socket - " + std::string(strerror(errno)));
			::close(sockfd); disabled = true; return (1);
		}
		Log.info("Unix Server: started successfully");

		return (0);
	}

#pragma endregion

#pragma region "Close"

	int UnixServer::close() {
		::close(sockfd);
		Log.debug("Unix Server: socket closed successfully");
		return (0);
	}

#pragma endregion
