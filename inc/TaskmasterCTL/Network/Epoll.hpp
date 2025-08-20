/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Epoll.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/13 11:17:16 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/14 21:12:50 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Epoll"

	class Socket;
	class Epoll {

		public:

			// Variables
			static bool Running;												// Indicates whether the main loop is running

			// Methods
			static int	create();												// Creates and initializes EPOLL and timeout
			static void	close();												// Closes EPOLL
			static int	add(int fd, bool epollin, bool epollout);				// Adds an event to EPOLL
			static int	set(int fd, bool epollin, bool epollout);				// Modifies an event in EPOLL
			static void	remove(int fd);											// Removes an event from EPOLL
			static int	events(Socket *socket);									// Processes EPOLL events

		private:

			// Variables
			static int			epoll_fd;										// File descriptor for EPOLL
			static int			timeout_fd;										// File descriptor used to generating events in EPOLL and checking clients timeout

			static const int 	MAX_EVENTS;										// Maximum number of events that can be handled per iteration by EPOLL
			static const int 	TIMEOUT_INTERVAL;								// Interval in seconds between timeout checks for inactive clients

			// Constructors
			Epoll() {}															// Default constructor (no instantiable)
			~Epoll() {}															// Destructor (no instantiable)

			// Methods
			static int	create_timeout();										// Creates the file descriptor for timeout
			static void check_timeout();										// Checks for time outs
	};

#pragma endregion
