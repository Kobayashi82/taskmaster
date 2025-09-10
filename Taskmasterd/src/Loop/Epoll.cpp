/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Epoll.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/18 11:54:24 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/10 18:33:40 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Taskmaster/Taskmaster.hpp"

	#include <unistd.h>															// close()
	#include <cstring>															// strerror()
	#include <sys/epoll.h>														//

#pragma endregion

#pragma region "Constructors"

	Epoll::Epoll() : _epoll_fd(-1) {}

	Epoll::~Epoll() {
		if (_epoll_fd != -1) ::close(_epoll_fd);
	}

#pragma endregion

#pragma region "Close"

	void Epoll::close() {
		if (_epoll_fd != -1) ::close(_epoll_fd);
		_epoll_fd = -1;
	}

#pragma endregion

#pragma region "Create"

	int Epoll::create() {
			if (_epoll_fd >= 0) close();

			_epoll_fd = epoll_create1(EPOLL_CLOEXEC);
			if (_epoll_fd == -1) {
				Log.critical("Epoll: failed to create epoll instance - " + std::string(strerror(errno)));
				return (1);
			}

			return (0);
	}

#pragma endregion

#pragma region "Remove"

	int Epoll::remove(int fd) {
		if (_epoll_fd < 0 || fd < 0) return (1);

		if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL)) {
			Log.error("Epoll: failed to set event - " + std::string(strerror(errno)));
			return (1);
		}

		return (0);
	}

#pragma endregion

#pragma region "Set"

	int Epoll::set(int fd, bool epollin, bool epollout) {
		if (_epoll_fd < 0 || fd < 0) return (1);

		struct epoll_event epoll_event;

		epoll_event.data.fd = fd;

		if		(epollin && epollout)	epoll_event.events = EPOLLIN | EPOLLOUT;
		else if	(epollin)				epoll_event.events = EPOLLIN;
		else if	(epollout)				epoll_event.events = EPOLLOUT;
		else							return (0);

		if (epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, fd, &epoll_event)) {
			Log.error("Epoll: failed to set event - " + std::string(strerror(errno)));
			return (1);
		}

		return (0);
	}

#pragma endregion

#pragma region "Add"

	int Epoll::add(int fd, bool epollin, bool epollout) {
		if (_epoll_fd < 0 || fd < 0) return (1);

		struct epoll_event epoll_event;

		epoll_event.data.fd = fd;

		if		(epollin && epollout)	epoll_event.events = EPOLLIN | EPOLLOUT;
		else if	(epollin)				epoll_event.events = EPOLLIN;
		else if	(epollout)				epoll_event.events = EPOLLOUT;
		else							return (1);

		if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, fd, &epoll_event)) {
			Log.error("Epoll: failed to add event - " + std::string(strerror(errno)));
			return (1);
		}

		return (0);
	}

#pragma endregion

#pragma region "Wait"

	int Epoll::wait() {
		if (_epoll_fd < 0) return (1);

		struct epoll_event	events[MAX_EVENTS];
		int					timeout = 100;

		int eventCount = epoll_wait(_epoll_fd, events, MAX_EVENTS, timeout);
		if (eventCount == -1) {
			Log.critical("Epoll: failed during execution - " + std::string(strerror(errno)));
			return (1);
		}

		for (int i = 0; i < eventCount; ++i) {

			// Signals
			if (events[i].data.fd == Signal::signal_fd) {
				if (Signal::process()) return (1);
				continue;
			}

			if (events[i].data.fd == tskm.unix_server.sockfd) {
				
			}
			// EventInfo * event = Event::get(events[i].data.fd);
			// if (!event) continue;

			if (events[i].events & EPOLLIN) {
				// switch (event->type) {
				// 	case SOCKET: 	{ Socket::accept(event);				break; }
				// 	case CLIENT: 	{ Communication::read_client(event);	break; }
				// 	case DATA: 		{ Communication::read_data(event);		break; }
				// 	case CGI: 		{ Communication::read_cgi(event);		break; }
				// }
			}

			// event = Event::get(events[i].data.fd);
			// if (!event) continue;

			if (events[i].events & EPOLLOUT) {
				// switch (event->type) {
				// 	case CLIENT: 	{ Communication::write_client(event);	break; }
				// 	case DATA: 		{										break; }
				// 	case CGI: 		{ Communication::write_cgi(event);		break; }
				// }
			}
		}

		return (0);
	}

#pragma endregion
