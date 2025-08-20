/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/13 11:17:01 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/19 19:26:58 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Taskmaster/Config/Options.hpp"
	#include "Taskmaster/Config/Logging.hpp"

	#include "Taskmaster/Security/Encryption.hpp"

	#include "Taskmaster/Main/Shell.hpp"

	#include "Taskmaster/Network/Client.hpp"
	#include "Taskmaster/Network/Epoll.hpp"
	#include "Taskmaster/Network/Communication.hpp"

	#include <unistd.h>															// For close()
	#include <ctime>															// For time() and difftime()
	#include <csignal>															// For kill()
	#include <algorithm>														// For std::find()
	#include <sys/socket.h>														// For send()

#pragma endregion

#pragma region "Variables"

	std::map <int, std::unique_ptr<Client>>	clients;							// FDs linked to their client
	std::map <int, Client *>				shells;								// FDs of shells linked to their client
	std::vector <int> 						pending_removals = {};				// List of FDs scheduled for removal
	std::vector <int> 						terminated_pids = {};				// List of PIDs that have terminated (signal-safe)

#pragma endregion

#pragma region "Constructors"

    Client::Client(int _fd, std::string _ip, int _port, int _sockfd) : fd(_fd), ip(_ip), port(_port), sock_fd(_sockfd), type(MSG) {
		last_activity = std::time(NULL); diying = false; user = ""; pass = ""; authenticated = false;
		shell_running = false; shell_pid = 0; master_fd = -1; slave_fd = -1; sock_fd = -1; encryption_index = 0;
		terminal_cols = 80; terminal_rows = 24;
	}

    Client::Client(const Client & src) : fd(src.fd), ip(src.ip), port(src.port), sock_fd(src.sock_fd), type(MSG) {
		last_activity = src.last_activity; diying = src.diying; user = src.user; pass = src.pass; authenticated = src.authenticated;
		shell_running = src.shell_running; shell_pid = src.shell_pid; master_fd = src.master_fd; slave_fd = src.slave_fd;
		write_buffer = src.write_buffer; write_sh_buffer = src.write_sh_buffer; encryption_index = src.encryption_index;
		terminal_cols = src.terminal_cols; terminal_rows = src.terminal_rows;
	}

	Client::~Client() {
		if (fd >= 0) close(fd);
		if (master_fd >= 0) close(master_fd);
		if (slave_fd >= 0) close(slave_fd);
	}

#pragma endregion

#pragma region "Overloads"

	Client & Client::operator=(const Client & rhs) {
        if (this != &rhs) {
			fd = rhs.fd; ip = rhs.ip; port = rhs.port;
			last_activity = rhs.last_activity; diying = rhs.diying; user = rhs.user; pass = rhs.pass; authenticated = rhs.authenticated;
			shell_running = rhs.shell_running; shell_pid = rhs.shell_pid; master_fd = rhs.master_fd; slave_fd = rhs.slave_fd;
			write_buffer = rhs.write_buffer; write_sh_buffer = rhs.write_sh_buffer; encryption_index = rhs.encryption_index;
			terminal_cols = rhs.terminal_cols; terminal_rows = rhs.terminal_rows;
		}
		return (*this);
    }

	bool Client::operator==(const Client & rhs) const {
		return (fd == rhs.fd);
	}

#pragma endregion

#pragma region "Time-Out"

	void Client::check_timeout(int interval) {
		time_t current_time = std::time(NULL);
		if (difftime(current_time, last_activity) > interval) {
			Log->info("Client: [" + ip + ":" + std::to_string(port) + "] connection time-out");

			std::string response;
			if (type == CLIENT) {
				if (!Options::disabledEncryption)	response = encrypt_with_index("\n\rConnection time-out\n", encryption_index);
				else								response = "\n\rConnection time-out\n";
			} else response = "Connection time-out\n";
			write_buffer.insert(write_buffer.end(), response.begin(), response.end());
			diying = true;
			Epoll::set(fd, true, true);
		}
	}

    void Client::update_last_activity() { last_activity = std::time(NULL); }

#pragma endregion

#pragma region "Remove"

	void Client::remove() {
		clients.erase(fd);
	}

#pragma endregion

#pragma region "Schedule Removal"

	void Client::schedule_removal() {
		if (std::find(pending_removals.begin(), pending_removals.end(), fd) == pending_removals.end()) {
			Log->debug("Client: [" + ip + ":" + std::to_string(port) + "] scheduled for deferred removal");
			pending_removals.push_back(fd);
		}
	}

#pragma endregion

#pragma region "Process Pending Removals"

	void process_pending_removals() {
		for (int fd : pending_removals) {
			auto it = clients.find(fd);
			if (it != clients.end()) {
				Log->debug("Client: [" + it->second->ip + ":" + std::to_string(it->second->port) + "] removing");

				Client* client = it->second.get();

				if (client->shell_running || client->shell_pid > 0 || client->master_fd >= 0) shell_close(client);

				Log->info("Client: [" + client->ip + ":" + std::to_string(client->port) + "] disconnected");
				Epoll::remove(client->fd);
				close(client->fd);
				client->fd = -1;
				clients.erase(it);
			}
		}
		pending_removals.clear();
	}

#pragma endregion

#pragma region "Process Terminated PIDs"

	void process_terminated_pids() {
		for (int pid : terminated_pids) {
			for (auto& client_pair : clients) {
				Client *client = client_pair.second.get();
				if (client && client->shell_pid == pid && client->shell_running && !client->diying) {
					Log->debug("Client: [" + client->ip + ":" + std::to_string(client->port) + "] shell process " + std::to_string(pid) + " terminated");

					client->shell_running = false;
					client->shell_pid = 0;

					if (client->master_fd >= 0) {
						shells.erase(client->master_fd);
						Epoll::remove(client->master_fd);
						close(client->master_fd);
						client->master_fd = -1;
					}

					if (client->slave_fd >= 0) {
						close(client->slave_fd);
						client->slave_fd = -1;
					}

					client->schedule_removal();
					break;
				}
			}
		}
		terminated_pids.clear();
	}

#pragma endregion
