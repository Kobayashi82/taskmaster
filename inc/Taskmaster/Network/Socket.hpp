/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/13 11:17:12 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/16 14:10:27 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Socket"

	class Socket {

		public:

			// Variables
			int		sockfd;														// File descriptor associated with the client
			int		port;														// Port number of the client

			// Constructors
			Socket();															// Default constructor
			Socket(int _port);													// Parameterized constructor
			Socket(const Socket & src);											// Copy constructor
			~Socket();															// Destructor

			// Overloads
			Socket &	operator=(const Socket & rhs);							// Assignment operator
			bool		operator==(const Socket & rhs) const;					// Equality operator

			// Methods
			int			create();												// Creates socket
			void		close();												// Closes socket
			int			accept();												// Accepts a new connection (client)

			static bool	is_port_free(int port);									// Checks if a port is available
	};

#pragma endregion
