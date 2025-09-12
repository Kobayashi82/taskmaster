/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Event.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/18 11:20:55 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/12 12:01:35 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include "Programs/Process.hpp"

	#include <cstdint>															// uint8_t
	#include <set>																// std::set
	#include <map>																// std::unordered_map

#pragma endregion

#pragma region "Enumerators"

	enum class EventType { SIGNAL, UNIX_SOCKET, INET_SOCKET, CLIENT, STD_MASTER, STD_IN, STD_OUT, STD_ERR };

#pragma endregion

#pragma region "Event Info"

	struct EventInfo {
		// Variables
		int						fd;
		EventType 				type;
		Process					*proc;
		bool					dead;

		std::set<int>			in;
		std::set<int>			out;

		std::vector<uint8_t>	read_buffer;
		std::vector<uint8_t>	write_buffer;

		// Constructors
		EventInfo() = delete;
		EventInfo(int _fd, EventType _type, Process *_proc) : fd(_fd), type(_type), proc(_proc), dead(false) {}
		EventInfo(const EventInfo&) = default;
		EventInfo(EventInfo&&) = default;
		~EventInfo() = default;

		// Overloads
		EventInfo& operator=(const EventInfo&) = default;
		EventInfo& operator=(EventInfo&&) = default;
	};

#pragma endregion

#pragma region "Event"

	class Event {

		public:

			// Variables
			std::unordered_map <int, EventInfo> events;

			// Events
			void		add(int fd, EventType type, Process *proc);
			EventInfo*	get(int fd);
			Process*	get_process(int fd);
			void		remove(int fd);
			void		remove_clients();
			void		clear();

			// In / Out
			void		in_remove(int fd, int from);
			void		in_add(int fd, int to);
			void		out_remove(int fd, int from);
			void		out_add(int fd, int to);

	};

#pragma endregion
