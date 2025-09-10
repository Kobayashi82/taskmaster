/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Event.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/18 11:20:55 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/10 19:59:36 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include <any>																// std::any
	#include <map>																// std::map

#pragma endregion

#pragma region "EventInfo"

	// Enumerators
	enum class EventType { NOTHING, SOCKET, CLIENT, DATA, CGI };

	struct EventInfo {
		int					fd;
		EventType 			type;
		std::any			owner;

		std::vector <char>	read_buffer;
		std::vector <char>	write_buffer;
	};

#pragma endregion

#pragma region "Event"

	class Event {

		public:

			// Variables
			std::unordered_map <int, EventInfo> events;

			// --- METHODS ---

			// Get Owner (template)
			template <typename T>
			T* get_owner(int fd) {
				if (fd < 0) return (nullptr);

				auto it = events.find(fd);
				if (it != events.end() && it->second.owner.has_value()) {
					try { return (std::any_cast<T*>(it->second.owner)); }
					catch (const std::bad_any_cast&) { return (nullptr); }
				}
				return (nullptr);
			}

			EventInfo*	get(int fd);
			EventInfo*	get_owner(int fd);
			void		clear();
			void		remove(int fd);
			void		remove(void *owner);
	};

#pragma endregion
