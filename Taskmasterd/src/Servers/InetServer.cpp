/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   InetServer.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/05 19:24:17 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/10 22:31:24 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Taskmaster/Taskmaster.hpp"

	#include <cstring>															// strerror()
	#include <unistd.h>															// gethostname(), close()
	#include <filesystem>														// std::filesistem::path()
	#include <sys/socket.h>														// socket()
	#include <netinet/in.h>														// struct sockaddr_in
	#include <netdb.h>															// getaddrinfo(), freeaddrinfo()
	#include <arpa/inet.h>														// inet_pton(), inet_ntoa()
	#include <sstream>															// std::stringstream

#pragma endregion

#pragma region "Constructors"

	InetServer::~InetServer() {
		close();
	}

#pragma endregion

#pragma region "Configuration"

	#pragma region "Validate"

		std::string InetServer::validate(const std::string& key, ConfigParser::ConfigEntry *entry) {
			if (key.empty() || !entry) return {};

			if (key == "password" && !entry->value.empty() && !Utils::valid_password(entry->value)) {
				Utils::error_add(entry->filename, "[" + section + "] " + key + ": invalid SHA format", ERROR, entry->line, entry->order + 2);
				entry->value = "";
			}

			return (entry->value);
		}

	#pragma endregion

	#pragma region "Expand Vars"

		std::string InetServer::expand_vars(std::map<std::string, std::string>& env, const std::string& key) {
			if (key.empty()) return {};

			ConfigParser::ConfigEntry *entry = Config.get_value_entry(section, key);
			if (!entry) return {};

			try {
				entry->value = Utils::environment_expand(env, entry->value);
				entry->value = Utils::remove_quotes(entry->value);
				if (entry->value.empty() && key != "username" && key != "password") {
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": empty value", ERROR, entry->line, entry->order);
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section][key], WARNING, entry->line, entry->order + 1);
					entry->value = Config.defaultValues[section][key];
				}
			}
			catch(const std::exception& e) {
				Utils::error_add(entry->filename, "[" + section + "] " + key + ": unclosed quote or unfinished escape sequence", ERROR, entry->line, entry->order);
				if (key != "username" && key != "password") {
					Utils::error_add(entry->filename, "[" + section + "] " + key + ": reset to default value: " + Config.defaultValues[section][key], WARNING, entry->line, entry->order + 1);
					entry->value = Config.defaultValues[section][key];
				}
			}

			return (validate(key, entry));
		}

	#pragma endregion

	#pragma region "Initialize"

		void InetServer::initialize() {
			std::string configFile;
			uint16_t	order = 0;
			disabled = false;
			sockfd = -1;
			port = 9001;

			section = "inet_http_server";
			if (!Config.has_section("inet_http_server")) { disabled = true; return; }

			ConfigParser::ConfigEntry *entry = Config.get_value_entry(section, "username");
			if (entry) { configFile = entry->filename; order = entry->order; }

			std::map<std::string, std::string> env;
			Utils::environment_clone(env, tskm.environment);

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
				} else if (entry->value.empty()) {
					Utils::error_add(entry->filename, "[" + section + "] port: empty value", ERROR, entry->line, entry->order);
					disabled = true;
				}
				if (!disabled) {
					url = entry->value;
					size_t pos = url.find_first_of(":");
					if (pos != std::string::npos) {
						host = url.substr(0, pos);
						port = Utils::parse_number(url.substr(pos + 1), 1, 65535, 9001);
					}
				}
			} else {
				if (!username.empty() && !password.empty())
					Utils::error_add(configFile, "[" + section + "] port: required", ERROR, 0, order);
				disabled = true;
			}
		}

	#pragma endregion

#pragma endregion

#pragma region "Resolve Host"

	std::string InetServer::resolve_host(const std::string& host) {
		if (host.empty() || host == "*") return ("0.0.0.0");

		struct sockaddr_in sa;
		if (inet_pton(AF_INET, host.c_str(), &(sa.sin_addr)) == 1) return (host);

		hostname = host;
		struct addrinfo hints, *result;
		std::memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;

		int status = getaddrinfo(host.c_str(), nullptr, &hints, &result);
		if (status) {
			Log.error("Inet Server: Failed to resolve hostname '" + host + "': " + std::string(gai_strerror(status)));
			return ("");
		}

		struct sockaddr_in* addr_in = (struct sockaddr_in *)result->ai_addr;
		std::string ip = inet_ntoa(addr_in->sin_addr);

		freeaddrinfo(result);

		return (ip);
	}

#pragma endregion

#pragma region "Start"

	int InetServer::start() {
		std::string ip = resolve_host(host);
		if (ip.empty()) { disabled = true; return (1); }

		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd == -1) {
			Log.error("Inet Server: failed to create socket - " + std::string(strerror(errno)));
			disabled = true;
			return (1);
		}
		Log.debug("Inet Server: socket created: " + std::to_string(sockfd));

		sockaddr_in addr;
		std::memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) != 1) {
			Log.error("Inet Server: invalid IP address format: " + ip);
			::close(sockfd); disabled = true;
			return (1);
		}

		int opt = 1;
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) Log.warning("Inet Server: failed to set SO_REUSEADDR");

		if (bind(sockfd, (sockaddr *)&addr, sizeof(addr)) == -1) {
			Log.error("Inet Server: failed to bind socket - " + std::string(strerror(errno)));
			::close(sockfd); disabled = true;
			return (1);
		}
		Log.debug("Inet Server: socket bound to " + (hostname.empty() ? ip : hostname) + ":" + std::to_string(port));

		if (listen(sockfd, 50) == -1) {
			Log.error("Inet Server: failed to listen on socket - " + std::string(strerror(errno)));
			::close(sockfd); disabled = true;
			return (1);
		}

		tskm.event.events.emplace(sockfd, EventInfo(sockfd, EventType::INET_SOCKET, this));
		if (tskm.epoll.add(sockfd, true, false)) {
			Log.error("Inet Server: failed to start");
			return (1);
		}

		Log.info("Inet Server: started on " + (hostname.empty() ? ip : hostname) + ":" + std::to_string(port));

		return (0);
	}

#pragma endregion

#pragma region "Close"

	void InetServer::close() {
		if (sockfd == -1) return;
		::close(sockfd); sockfd = -1;
		Log.debug("Inet Server: socket closed");
	}

#pragma endregion

#pragma region "Accept"

	int InetServer::accept() {
		if (sockfd == -1) return (-1);

		sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);
		
		int client_fd = ::accept4(sockfd, (sockaddr*)&client_addr, &client_len, SOCK_CLOEXEC | SOCK_NONBLOCK);
		if (client_fd == -1) {
			if (errno != EAGAIN && errno != EWOULDBLOCK)
				Log.error("Inet Server: failed to accept client - " + std::string(strerror(errno)));
			return (-1);
		}

		std::string	client_ip	= inet_ntoa(client_addr.sin_addr);
		uint16_t	client_port	= ntohs(client_addr.sin_port);

		Log.info("Inet Server: client connected — " + client_ip + ":" + std::to_string(client_port) + " (fd: " + std::to_string(client_fd) + ")");

		tskm.event.events.emplace(client_fd, EventInfo(client_fd, EventType::CLIENT, this));

		if (tskm.epoll.add(client_fd, true, false) == -1) {
			Log.error("Inet Server: client disconnected — " + client_ip + ":" + std::to_string(client_port) + " (fd: " + std::to_string(client_fd) + ")");
			::close(client_fd);
			tskm.event.remove(client_fd);
			return (-1);
		}

		return (client_fd);
	}

#pragma endregion
