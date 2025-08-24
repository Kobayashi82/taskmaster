/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Communication.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/13 21:46:18 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/24 16:27:54 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include <string>															// std::string

#pragma endregion

#pragma region "Communication"

	class Client;
	class Communication {

		private:
	
			//	Variables
			static const size_t			CHUNK_SIZE;								// Size of the buffer for read/recv and write/send operations

			// Constructors
			Communication() {}													// Default constructor (no instantiable)
			~Communication() {}													// Destructor (no instantiable)

		public:

			//	Methods
			static int	read_client(Client *client);							// Reads data from the client socket
			static void	write_client(Client *client);							// Writes data to the client socket

			static int	read_shell(Client *client);								// Reads data from the shell associated with the client
			static void	write_shell(Client *client);							// Writes data to the shell associated with the client
	};

#pragma endregion
