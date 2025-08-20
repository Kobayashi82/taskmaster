/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/13 11:17:06 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/20 14:01:21 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Options.hpp"
	#include "Config/Logging.hpp"

	#include "Network/Socket.hpp"
	#include "Network/Client.hpp"
	#include "Network/Epoll.hpp"

	#include <arpa/inet.h>														// socket(), setsockopt(), bind(), listen(), accept(), inet_ntop(), htons(), ntohs(), sockaddr_in
	#include <sys/socket.h>														// send()
	#include <unistd.h>															// close()
	#include <cstring>															// std::memset()

#pragma endregion

#pragma region "Constructors"

    Socket::Socket() : sockfd(-1), port(4242) {}

    Socket::Socket(int _port) : sockfd(-1), port(_port) {}

    Socket::Socket(const Socket & src) : sockfd(src.sockfd), port(src.port) {}

	Socket::~Socket() { close(); }

#pragma endregion

#pragma region "Overloads"

	Socket & Socket::operator=(const Socket & rhs) {
        if (this != &rhs) { sockfd = rhs.sockfd; port = rhs.port; }
		return (*this);
    }

	bool Socket::operator==(const Socket & rhs) const {
		return (sockfd == rhs.sockfd);
	}

#pragma endregion

#pragma region "Create"

	int Socket::create() {
		if (sockfd != -1) return (1);

		// Create socket
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0) {
			Log->critical("Daemon: Socket creation failed");
			return (1);
		}
		Log->debug("Daemon: Socket created");

		// Configure socket
		int options = 1;
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &options, sizeof(options)) == -1) {
			Log->critical("Daemon: Socket set option failed");
			::close(sockfd); return (1);
		}
		Log->debug("Daemon: Socket reusable option set");

		// Initialize the socket address structure
		sockaddr_in address; std::memset(&address, 0, sizeof(address));
		address.sin_family = AF_INET;
		address.sin_port = htons(port);
		address.sin_addr.s_addr = INADDR_ANY;

		// Link the address to the socket
		if (bind(sockfd, (sockaddr *)&address, sizeof(address)) == -1) {
			Log->critical("Daemon: Socket bind failed");
			::close(sockfd); return (1);
		}
		Log->debug("Daemon: Socket bind set");

		// Listen on the address for incoming connections
		if (listen(sockfd, SOMAXCONN) == -1) {
			Log->critical("Daemon: Socket listen failed");
			::close(sockfd); return (1);
		}
		Log->debug("Daemon: Socket listen set");

		// Add the socket FD to EPOLL
		if (Epoll::add(sockfd, true, false) == -1) {
			Log->critical("Daemon: Epoll Socket FD add failed");
			::close(sockfd); return (1);
		}
		Log->debug("Daemon: Socket added to Epoll");

		Options::sockfd = sockfd;
		return (0);
	}

#pragma endregion

#pragma region "Close"

	void Socket::close() {
		if (sockfd != -1) {
			Log->debug("Daemon: Socket close");
			::close(sockfd); clients.clear();
		}
	}

#pragma endregion

#pragma region "Accept"

	int Socket::accept() {
		sockaddr_in Addr; socklen_t AddrLen = sizeof(Addr);
		int fd = ::accept(sockfd, (sockaddr *)&Addr, &AddrLen);
		if (fd < 0) { Log->critical("Daemon: Socket accept connection failed"); return (1); }

		char ip_str[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &(Addr.sin_addr), ip_str, INET_ADDRSTRLEN);
		std::string ip	= ip_str;
		int	port		= ntohs(Addr.sin_port);	

		clients.emplace(fd, std::make_unique<Client>(fd, ip, port, sockfd));

		if (clients.size() > Options::maxClients && Options::maxClients) {
			Log->warning("Client: [" + ip + ":" + std::to_string(port) + "] denied. Maximum clients reached");	
			Client *client = nullptr; 
			auto it = clients.find(fd);
			if (it != clients.end()) client = it->second.get();

			if (client) {
				std::string msg = "Maximum connections reached\n";

				send(fd, msg.c_str(), msg.length(), 0);

				::close(fd);
				clients.erase(fd);

				return (0);
			}
		} else {
			Log->info("Client: [" + ip + ":" + std::to_string(port) + "] connected");
			
			if (Epoll::add(fd, true, false) == -1) {
				Log->debug("Daemon: Epoll FD add failed");
				clients[fd]->remove(); return (1);
			}
			Log->debug("Client: [" + ip + ":" + std::to_string(port) + "] added to Epoll");
		}

		return (0);
	}

#pragma endregion

#pragma region "Is Port Free"

	bool Socket::is_port_free(int port) {
		int sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock < 0) return (false);

		int options = 1;
		if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &options, sizeof(options)) == -1) {
			::close(sock); return (false);
		}

		sockaddr_in addr{};
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		addr.sin_port = htons(port);

		bool isFree = (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0);
		::close(sock);

		return (isFree);
	}

#pragma endregion
