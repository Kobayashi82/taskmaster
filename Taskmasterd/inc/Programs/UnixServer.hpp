/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UnixServer.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/05 19:26:41 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/05 21:42:05 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include <cstdint>															// uint16_t
	#include <string>															// std::string
	#include <map>																// std::map
	#include <vector>															// std::vector

#pragma endregion

#pragma region "UnixServer"

	class UnixServer {

		public:

			// Constructors
			UnixServer() = default;
			UnixServer(const UnixServer&) = default;
			UnixServer(UnixServer&&) = default;
			~UnixServer() = default;

			// Overloads
			UnixServer& operator=(const UnixServer&) = default;
			UnixServer& operator=(UnixServer&&) = default;

			// Variables
			std::string		section;
			std::string		file;
			uint16_t		chmod;
			std::string		chown_user;
			std::string		chown_group;
			std::string		username;
			std::string		password;
			bool			disabled;

			void		initialize();
		private:

			std::string	validate(const std::string& key, ConfigParser::ConfigEntry *entry);
			std::string	expand_vars(std::map<std::string, std::string>& env, const std::string& key);

	};

#pragma endregion
