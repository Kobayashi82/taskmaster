/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UnixServer.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/05 19:24:11 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/07 23:37:40 by vzurera-         ###   ########.fr       */
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
	#include <sys/stat.h>														// chmod()
	#include <pwd.h>															// struct passwd, getpwuid(), getpwnam()
	#include <grp.h>															// getgrgid()
	#include <filesystem>														// std::filesistem::path()
	#include <sstream>															// std::stringstream
	#include <sys/socket.h>														// socket()
	#include <sys/un.h>															// struct sockaddr_un
	#include <iostream>

#pragma endregion

#pragma region "Constructors"

	UnixServer::~UnixServer() {
		close();
	}

#pragma endregion

#pragma region "Initialize"

	#pragma region "Validate"

		std::string UnixServer::validate(const std::string& key, ConfigParser::ConfigEntry *entry) {
			if (key.empty() || !entry) return {};

			if (key == "chmod" && !Utils::valid_chmod(entry->value)) {
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
			chown_uid = -1;
			chown_gid = -1;

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
			if (chown_str.empty())				disabled = true;
			else if (!resolve_user(chown_str))	disabled = true;

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

#pragma region "Resolve User"

	bool UnixServer::resolve_user(const std::string& value) {
		if (value.empty()) {
			chown_uid = -1;
			chown_gid = -1;
			return (false);
		}

		if (Utils::toLower(value) == "do not switch") {
			chown_uid = getuid();
			chown_gid = getgid();
			return (true);
		}

		std::string	user_part, group_part;
		size_t		colon_pos = value.find(':');

		if (colon_pos != std::string::npos) {
			user_part  = value.substr(0, colon_pos);
			group_part = value.substr(colon_pos + 1);
		} else
			user_part = value;

		struct passwd *pw = nullptr; char *endptr = nullptr; errno = 0;
		long uid = std::strtol(user_part.c_str(), &endptr, 10);
		if (!*endptr && errno == 0 && uid >= 0 && uid <= INT_MAX)	pw = getpwuid(static_cast<uid_t>(uid));
		else														pw = getpwnam(user_part.c_str());
		chown_uid = pw->pw_uid;

		if (group_part.empty())
			chown_gid = pw->pw_gid;
		else {
			struct group *gr = nullptr; errno = 0;
			long gid = std::strtol(group_part.c_str(), &endptr, 10);
			if (!*endptr && errno == 0 && gid >= 0 && gid <= INT_MAX)	gr = getgrgid(static_cast<gid_t>(gid));
			else														gr = getgrnam(group_part.c_str());

			if (!gr) { chown_gid = -1; return (false); }
			chown_gid = gr->gr_gid;
		}

		return (true);
	}

#pragma endregion

#pragma region "Apply Ownership"

	bool UnixServer::apply_ownership(uid_t uid, gid_t gid) {
		if (uid == getuid() && gid == getgid()) return (true);

		if (uid != static_cast<uid_t>(-1) || gid != static_cast<gid_t>(-1)) {
			if (chown(file.c_str(), uid, gid) == -1) {
				Log.warning("Unix Server: unable to change socket ownership - " + std::string(strerror(errno)));
				chown_uid = chown_gid = -2; return (false);
			}

			struct passwd *pw = getpwuid(uid);
			struct group *gr = getgrgid(gid);

			std::string user_name = pw ? pw->pw_name : std::to_string(uid);
			std::string group_msg;
			if (gr) group_msg = "and group '" + std::string(gr->gr_name) + "' (GID:" + std::to_string(gid) + ")";
			else    group_msg = "and GID:" + std::to_string(gid);

			Log.debug("Unix Server: changed socket ownership to '" + user_name + "' (UID:" + std::to_string(uid) + ") " + group_msg);
		}

		return (true);
	}

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

		if (::chmod(file.c_str(), chmod) == -1) {
			Log.warning("Unix Server: failed to set socket permissions: " + std::string(strerror(errno)));
		}

		apply_ownership(chown_uid, chown_gid);

		if (listen(sockfd, 50) == -1) {
			Log.error("Unix Server: failed to listen on socket - " + std::string(strerror(errno)));
			::close(sockfd); disabled = true; return (1);
		}
		Log.info("Unix Server: started successfully");

		return (0);
	}

#pragma endregion

#pragma region "Close"

	void UnixServer::close() {
		if (sockfd == -1) return;
		::close(sockfd); sockfd = -1;
		Log.debug("Unix Server: socket closed successfully");
	}

#pragma endregion
