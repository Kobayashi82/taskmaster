/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Event.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/18 11:21:01 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/13 17:58:23 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Taskmaster/Taskmaster.hpp"

#pragma endregion

#pragma region "Events"

	#pragma region "Add"

		void Event::add(int fd, EventType type, Process *proc){
			events.emplace(fd, EventInfo(fd, type, proc));
		}

	#pragma endregion

	#pragma region "Get"

		#pragma region "Event"

			EventInfo* Event::get(int fd) {
				if (fd < 0) return (nullptr);

				auto it = events.find(fd);
				if (it != events.end())	return (&it->second);

				return (nullptr);
			}

		#pragma endregion

		#pragma region "Process"

			Process* Event::get_process(int fd) {
				if (fd < 0) return (nullptr);

				auto it = events.find(fd);
				if (it != events.end()) return (it->second.proc);

				return (nullptr);
			}

		#pragma endregion

	#pragma endregion

	#pragma region "Remove"

		#pragma region "Event"

			void Event::remove(int fd) {
				events.erase(fd);
			}

		#pragma endregion

		#pragma region "Clients"

			void Event::remove_clients() {
				for (auto it = events.begin(); it != events.end();) {
					if (it->second.type == EventType::CLIENT) {
						EventInfo event = it->second;
						tskm.epoll.remove(event.fd);
						::close(event.fd);
						for (auto& fd : event.in)	tskm.event.in_remove(event.fd, fd);
						for (auto& fd : event.out)	tskm.event.out_remove(event.fd, fd);
						it = events.erase(it);
						continue;
					}
					++it;
				}
			}

		#pragma endregion

		#pragma region "Clear"

			void Event::clear() {
				events.clear();
			}

		#pragma endregion

		#pragma region "Close & Clear"

			void Event::close_clear() {
				for (auto it = events.begin(); it != events.end();) {
					EventInfo event = it->second;
					tskm.epoll.remove(event.fd);
					::close(event.fd);
					it = events.erase(it);
				}
			}

		#pragma endregion

	#pragma endregion

#pragma endregion

#pragma region "In / Out"

	#pragma region "In"

		#pragma region "Remove"

			void Event::in_remove(int fd, int from) {
				if (fd < 0 || from < 0) return;

				auto it = events.find(from);
				if (it != events.end()) it->second.out.erase(fd);
			}

		#pragma endregion

		#pragma region "Add"

			void Event::in_add(int fd, int to) {
				if (fd < 0 || to < 0) return;

				auto it = events.find(to);
				if (it != events.end()) it->second.out.insert(fd);
			}

		#pragma endregion

	#pragma endregion

	#pragma region "Out"

		#pragma region "Remove"

			void Event::out_remove(int fd, int from) {
				if (fd < 0 || from < 0) return;

				auto it = events.find(from);
				if (it != events.end()) it->second.out.erase(fd);
			}

		#pragma endregion

		#pragma region "Add"

			void Event::out_add(int fd, int to) {
				if (fd < 0 || to < 0) return;

				auto it = events.find(to);
				if (it != events.end()) it->second.out.insert(fd);
			}

		#pragma endregion

	#pragma endregion

#pragma endregion
