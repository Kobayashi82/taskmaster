/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Epoll.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/18 11:54:24 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/11 13:41:15 by vzurera-         ###   ########.fr       */
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

			Log.debug("Epoll: epoll create: " + std::to_string(_epoll_fd));

			return (0);
	}

#pragma endregion

#pragma region "Remove"

	int Epoll::remove(int fd) {
		if (_epoll_fd < 0 || fd < 0) return (1);

		if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL)) {
			Log.error("Epoll: failed to set event for fd " +  std::to_string(fd) + " - " + std::string(strerror(errno)));
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
			Log.error("Epoll: failed to set event for fd " +  std::to_string(fd) + " - " + std::string(strerror(errno)));
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
			Log.error("Epoll: failed to add event for fd " +  std::to_string(fd) + " - " + std::string(strerror(errno)));
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

			EventInfo *event = tskm.event.get(events[i].data.fd);
			if (!event) continue;

			if (!event->dead && events[i].events & EPOLLIN) {
				switch (event->type) {
					case EventType::SIGNAL:			{ if (Signal::process()) return (1);	break; }
					case EventType::UNIX_SOCKET:	{ tskm.unix_server.accept();			break; }
					case EventType::INET_SOCKET:	{ tskm.inet_server.accept();			break; }
					case EventType::CLIENT:			{ 										break; }
					case EventType::STD_MASTER:		{ 										break; }
					case EventType::STD_IN:			{ 										break; }
					case EventType::STD_OUT:		{ 										break; }
					case EventType::STD_ERR:		{ 										break; }
				}
			}

			if (!event->dead && events[i].events & EPOLLOUT) {
				switch (event->type) {
					case EventType::SIGNAL:			{ 										break; }
					case EventType::UNIX_SOCKET:	{ 										break; }
					case EventType::INET_SOCKET:	{ 										break; }
					case EventType::CLIENT:			{ 										break; }
					case EventType::STD_MASTER:		{ 										break; }
					case EventType::STD_IN:			{ 										break; }
					case EventType::STD_OUT:		{ 										break; }
					case EventType::STD_ERR:		{ 										break; }
				}
			}

			if (event->dead) {
				remove(event->fd);
				::close(event->fd);
				for (auto& fd : event->in)	tskm.event.in_remove(event->fd, fd);
				for (auto& fd : event->out)	tskm.event.out_remove(event->fd, fd);
				tskm.event.remove(event->fd);
			}
		}

		return (0);
	}

#pragma endregion
