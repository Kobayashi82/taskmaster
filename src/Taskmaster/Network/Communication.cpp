/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Communication.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/13 21:46:19 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/20 12:14:18 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Taskmaster/Config/Options.hpp"
	#include "Taskmaster/Config/Logging.hpp"

	#include "Taskmaster/Security/Authentication.hpp"
	#include "Taskmaster/Security/Encryption.hpp"

	#include "Taskmaster/Main/Shell.hpp"

	#include "Taskmaster/Network/Socket.hpp"
	#include "Taskmaster/Network/Client.hpp"
	#include "Taskmaster/Network/Epoll.hpp"
	#include "Taskmaster/Network/Communication.hpp"

	#include <cstring>															// std::memset()
	#include <unistd.h>															// read(), write()
	#include <sys/socket.h>														// recv(), send()
	#include <sys/ioctl.h>														// ioctl()
	#include <termios.h>														// struct winsize

#pragma endregion

#pragma region "Variables"

	const size_t Communication::CHUNK_SIZE	= 4096;								// Size of the buffer for read/recv and write/send operations

#pragma endregion

#pragma region "Communications"

	#pragma region "CLIENT"

		#pragma region "Read"

			int Communication::read_client(Client *client) {
				if (!client || client->fd < 0 || client->diying) return (0);

				char buffer[CHUNK_SIZE];	std::memset(buffer, 0, sizeof(buffer));
				ssize_t bytes_read = recv(client->fd, buffer, CHUNK_SIZE, 0);

				// Read some data
				if (bytes_read > 0) {
					client->update_last_activity();

					if (client->type == MSG) {
						std::string msg = std::string(buffer, buffer + bytes_read);
						if (!msg.empty() && msg.back() == '\n') msg.pop_back();
						if (msg == "quit") {
							Log->warning("Client: [" + client->ip + ":" + std::to_string(client->port) + "] wants to close the daemon");
							Epoll::Running = false;

							return (0);
						}

						if (msg == "/CLIENT_SHELL_AUTH") {
							Log->info("Client: [" + client->ip + ":" + std::to_string(client->port) + "] wants to open a shell");
							std::string response;
							client->type = CLIENT;

							if (Options::disabledShell) {
								Log->warning("Client: [" + client->ip + ":" + std::to_string(client->port) + "] shell is disabled, rejecting connection");
								response = "/SHELL_DISABLED\n";
								client->diying = true;
							} else {
								response = "/AUTHORIZE ENCRYPTION=" + std::string(!Options::disabledEncryption ? "true\n" : "false\n");
								client->authenticated = false;
							}

							client->write_buffer.insert(client->write_buffer.end(), response.begin(), response.end());
							Epoll::set(client->fd, true, true);

							return (0);
						}

						if (msg.find_first_not_of(" \t\n\r") != std::string::npos) Log->log("Input: " + msg);
						return (0);
					}

					if (client->type == CLIENT) {
						std::string msg = std::string(buffer, buffer + bytes_read);
						try {
							if (!Options::disabledEncryption) msg = decrypt(msg);
						} catch (const std::exception& e) {
							Log->warning("Client: [" + client->ip + ":" + std::to_string(client->port) + "] message not encrypted, rejecting connection");
							client->diying = true;
							std::string response = "message not encrypted, rejecting connection\n";
							client->write_buffer.insert(client->write_buffer.end(), response.begin(), response.end());
							Epoll::set(client->fd, true, true);

							return (1);
						}

						if (!client->shell_running && msg.substr(0, 14) == "/AUTHORIZATION") {
							std::string user, pass, response = "/AUTHORIZATION_FAIL\n";
							client->authenticated = false;

							if (!msg.empty() && msg.back() == '\n') msg.pop_back();
							if (get_userpass(msg.substr(14), user, pass)) {
								Log->warning("Client: [" + client->ip + ":" + std::to_string(client->port) + "] invalid authorization format");
							} else if (authenticate(user, pass)) {
								Log->info("Client: [" + client->ip + ":" + std::to_string(client->port) + "] authorization successful for user: " + user);
								response = "/AUTHORIZATION_OK\n";
								client->authenticated = true;
								client->user = user;
								client->pass = pass;
							} else Log->warning("Client: [" + client->ip + ":" + std::to_string(client->port) + "] authorization failed for user: " + user);

							if (!Options::disabledEncryption) response = encrypt(response);
							client->write_buffer.insert(client->write_buffer.end(), response.begin(), response.end());
							Epoll::set(client->fd, true, true);

							return (0);
						}

						if (msg.substr(0, 14) == "/TERMINAL_SIZE") {
							std::string	size_info = msg.substr(15);
							size_t		x_pos = size_info.find('x');

							if (x_pos != std::string::npos) {
								try {
									client->terminal_cols = std::stoi(size_info.substr(0, x_pos));
									client->terminal_rows = std::stoi(size_info.substr(x_pos + 1));
									Log->debug("Client: [" + client->ip + ":" + std::to_string(client->port) + "] terminal size: " + std::to_string(client->terminal_cols) + "x" + std::to_string(client->terminal_rows));

									// If authenticated, shell not running and client not dying, start shell
									if (client->type == CLIENT && client->authenticated && !client->shell_running && !client->diying && !shell_start(client)) return (0);

								} catch (const std::exception& e) { Log->warning("Client: [" + client->ip + ":" + std::to_string(client->port) + "] invalid terminal size format"); }
							}

							std::string response = "/SHELL_FAIL\n";
							if (!Options::disabledEncryption) response = encrypt(response);
							client->write_buffer.insert(client->write_buffer.end(), response.begin(), response.end());
							client->diying = true;
							Epoll::set(client->fd, true, true);

							return (1);
						}

						if (msg.substr(0, 16) == "/TERMINAL_RESIZE") {
							std::string	size_info = msg.substr(17);
							size_t		x_pos = size_info.find('x');

							if (x_pos != std::string::npos) {
								try {
									client->terminal_cols = std::stoi(size_info.substr(0, x_pos));
									client->terminal_rows = std::stoi(size_info.substr(x_pos + 1));

									if (client->shell_running && client->master_fd >= 0) {
										struct winsize ws;
										ws.ws_row = client->terminal_rows;
										ws.ws_col = client->terminal_cols;
										ws.ws_xpixel = 0;
										ws.ws_ypixel = 0;
										if (!ioctl(client->master_fd, TIOCSWINSZ, &ws)) {
											Log->debug("Client: [" + client->ip + ":" + std::to_string(client->port) + "] terminal resized to: " + std::to_string(client->terminal_cols) + "x" + std::to_string(client->terminal_rows));
										} else {
											Log->warning("Client: [" + client->ip + ":" + std::to_string(client->port) + "] failed to resize PTY");
										}
									}
								} catch (const std::exception& e) { Log->warning("Client: [" + client->ip + ":" + std::to_string(client->port) + "] invalid terminal resize format"); }
							}

							return (0);
						}

						if (client->master_fd == -1) {
							std::string response = "/DISCONNECT\n";
							if (!Options::disabledEncryption) response = encrypt(response);
							client->write_buffer.insert(client->write_buffer.end(), response.begin(), response.end());
							client->diying = true;
							Epoll::set(client->fd, true, true);

							return (1);
						}

						client->write_sh_buffer.insert(client->write_sh_buffer.end(), msg.begin(), msg.end());
						if (client->shell_running && client->master_fd >= 0) Epoll::set(client->master_fd, true, true);
					}
				}

				// No data
				else if (bytes_read == 0) { client->schedule_removal(); return (1); }
				// Error reading
				else if (bytes_read == -1) { client->schedule_removal(); return (1); }

				return (0);
			}

		#pragma endregion

		#pragma region "Write"

			void Communication::write_client(Client *client) {
				if (!client || client->fd < 0) return;

				// There are data, send it
				if (!client->write_buffer.empty()) {
					client->update_last_activity();

					size_t buffer_size = client->write_buffer.size();
					size_t chunk = std::min(buffer_size, CHUNK_SIZE);
					ssize_t bytes_written = send(client->fd, client->write_buffer.data(), chunk, 0);

					// Send some data
					if (bytes_written > 0) {
						client->write_buffer.erase(client->write_buffer.begin(), client->write_buffer.begin() + bytes_written);
						if (client->write_buffer.empty()) Epoll::set(client->fd, true, false);
					}

					// Client dying
					if (client->diying && client->write_buffer.empty()) { client->schedule_removal(); return; }
					// No writing
					else if (bytes_written == 0) { client->schedule_removal(); return; }
					// Error writing
					else if (bytes_written == -1) { client->schedule_removal(); return; }
				}
			}

		#pragma endregion

	#pragma endregion

	#pragma region "SHELL"

		#pragma region "Read"

			int Communication::read_shell(Client *client) {
				if (!client || client->master_fd < 0 || !client->shell_running) return (0);

				char buffer[CHUNK_SIZE];	std::memset(buffer, 0, sizeof(buffer));
				ssize_t bytes_read = read(client->master_fd, buffer, CHUNK_SIZE);

				// Read some data
				if (bytes_read > 0) {
					client->update_last_activity();
					std::string output(buffer, bytes_read);
					if (!Options::disabledEncryption) output = encrypt_with_index(output, client->encryption_index);
					client->write_buffer.insert(client->write_buffer.end(), output.begin(), output.end());
					Epoll::set(client->fd, true, true);
				}

				// No data
				else if (bytes_read == 0) return (0);
				// Error reading
				else if (bytes_read == -1) return (0);

				return (0);
			}

		#pragma endregion

		#pragma region "Write"

			void Communication::write_shell(Client *client) {
				if (!client || client->master_fd < 0 || !client->shell_running) return;

				// There are data, send it
				if (!client->write_sh_buffer.empty()) {
					client->update_last_activity();

					size_t buffer_size = client->write_sh_buffer.size();
					size_t chunk = std::min(buffer_size, CHUNK_SIZE);

					std::string input(client->write_sh_buffer.begin(), client->write_sh_buffer.begin() + chunk);
					ssize_t bytes_written = write(client->master_fd, input.c_str(), input.length());

					// Send some data
					if (bytes_written > 0) {
						client->write_sh_buffer.erase(client->write_sh_buffer.begin(), client->write_sh_buffer.begin() + bytes_written);
						if (client->write_sh_buffer.empty()) Epoll::set(client->master_fd, true, false);
					}

					// No writing
					else if (bytes_written == 0) return;
					// Error writing
					else if (bytes_written == -1) return;
				}
			}

		#pragma endregion

	#pragma endregion

#pragma endregion
