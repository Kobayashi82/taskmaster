/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Epoll.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/13 11:16:51 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/20 14:01:21 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Options.hpp"
	#include "Config/Logging.hpp"

	#include "Network/Socket.hpp"
	#include "Network/Client.hpp"
	#include "Network/Epoll.hpp"
	#include "Network/Communication.hpp"

	#include <cstring>															// std::memset()
	#include <unistd.h>															// read(), close()
	#include <sys/epoll.h>														// epoll_create(), epoll_ctl(), epoll_wait(), epoll_event
	#include <sys/timerfd.h>													// timerfd_create(), timerfd_settime()

#pragma endregion

#pragma region "Variables"

	bool				Epoll::Running			= false;						// Indicates whether the main loop is running

	int					Epoll::epoll_fd			= -1;							// File descriptor for epoll
	int					Epoll::timeout_fd		= -1;							// File descriptor used to generating events in EPOLL and checking clients timeout

	const int			Epoll::MAX_EVENTS		= 10;							// Maximum number of events that can be handled per iteration by epoll
	const int			Epoll::TIMEOUT_INTERVAL	= 1;							// Interval in seconds between timeout checks for inactive clients

#pragma endregion

#pragma region "EPOLL"

	#pragma region "Time-Out"

		#pragma region "Create"

			int Epoll::create_timeout() {
				timeout_fd = timerfd_create(CLOCK_MONOTONIC, 0);
				if (timeout_fd == -1) return (1);

				struct itimerspec new_value;
				std::memset(&new_value, 0, sizeof(new_value));
				new_value.it_value.tv_sec = TIMEOUT_INTERVAL;					// Time to the first expiration
				new_value.it_interval.tv_sec = TIMEOUT_INTERVAL;				// Interval between expirations

				if (timerfd_settime(timeout_fd, 0, &new_value, NULL) == -1) {
					Log->warning("Timeout FD creation failed");
					::close(timeout_fd); timeout_fd = -1; return (1);
				}

				return (0);
			}

		#pragma endregion

		#pragma region "Check"

			void Epoll::check_timeout() {
				uint64_t expirations;
				read(timeout_fd, &expirations, sizeof(expirations));

				for (auto& pair : clients) pair.second->check_timeout(Options::timeout);
			}

		#pragma endregion

	#pragma endregion

	#pragma region "Create"

		int Epoll::create() {
			if (epoll_fd != -1) close();

			epoll_fd = epoll_create(1024);
			if (epoll_fd == -1) {
				Log->critical("Epoll creation failed");
				return (1);
			}
			if (!create_timeout()) add(timeout_fd, true, false);

			return (0);
		}

	#pragma endregion

	#pragma region "Close"

		void Epoll::close() {
			if (timeout_fd != -1)	{ ::close(timeout_fd);	timeout_fd = -1; }
			if (epoll_fd != -1)		{ ::close(epoll_fd);	epoll_fd = -1;	 }
		}

	#pragma endregion
	
	#pragma region "Add"

		int Epoll::add(int fd, bool epollin, bool epollout) {
			if (fd < 0) return (1);
			struct epoll_event epoll_event;

			epoll_event.data.fd = fd;

			if (epollin && epollout)	epoll_event.events = EPOLLIN | EPOLLOUT;
			else if (epollin) 			epoll_event.events = EPOLLIN;
			else if (epollout) 			epoll_event.events = EPOLLOUT;
			else { 						return (1); }

			int result = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &epoll_event);

			return (result);
		}

	#pragma endregion

	#pragma region "Set"

		int Epoll::set(int fd, bool epollin, bool epollout) {
			if (fd < 0) return (1);
			struct epoll_event epoll_event;

			epoll_event.data.fd = fd;

			if (epollin && epollout)	epoll_event.events = EPOLLIN | EPOLLOUT;
			else if (epollin) 			epoll_event.events = EPOLLIN;
			else if (epollout)			epoll_event.events = EPOLLOUT;
			else {				 		return (1); }

			return (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &epoll_event));
		}

	#pragma endregion

	#pragma region "Del"

		void Epoll::remove(int fd) {
			if (epoll_fd < 0 || fd < 0) return;
			epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
		}

	#pragma endregion

	#pragma region "Events"

		int Epoll::events(Socket *socket) {
			if (epoll_fd < 0) return (1);
			struct epoll_event events[MAX_EVENTS];

			int eventCount = epoll_wait(epoll_fd, events, MAX_EVENTS, 100);
			if (eventCount == -1) {
				if (errno == EINTR) return (0);
				if (Running) { Log->critical("Epoll failed");  return (1); }
			}

			for (int i = 0; i < eventCount; ++i) {
				if (events[i].data.fd == timeout_fd && Options::timeout)				{ check_timeout();	continue; }
				if (events[i].data.fd == socket->sockfd && events[i].events & EPOLLIN)	{ socket->accept();	continue; }

				Client *client = nullptr; int type = 0;
				auto it = clients.find(events[i].data.fd);
				if (it != clients.end()) { client = it->second.get(); type = client->type; }
				else {
					auto it = shells.find(events[i].data.fd);
					if (it != shells.end()) { client = it->second;	type = SHELL; }
				}

				if (!client) continue;

				if (events[i].events & EPOLLIN) {
					switch (type) {
						case MSG:	 	{ Communication::read_client(client);	break; }
						case CLIENT: 	{ Communication::read_client(client);	break; }
						case SHELL:	 	{ Communication::read_shell(client);	break; }
					}
				}

				if (events[i].events & EPOLLOUT) {
					switch (type) {
						case MSG: 		{ Communication::write_client(client);	break; }
						case CLIENT: 	{ Communication::write_client(client);	break; }
						case SHELL:	 	{ Communication::write_shell(client);	break; }
					}
				}
			}

			process_terminated_pids();
			process_pending_removals();

			return (0);
		}

	#pragma endregion

#pragma endregion
