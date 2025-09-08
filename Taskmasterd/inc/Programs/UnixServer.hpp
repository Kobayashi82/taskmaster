/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UnixServer.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/05 19:26:41 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/08 17:46:59 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include <string>															// std::string
	#include <cstdint>															// uint16_t
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
			~UnixServer();

			// Overloads
			UnixServer& operator=(const UnixServer&) = default;
			UnixServer& operator=(UnixServer&&) = default;

			// Variables
			std::string		section;
			std::string		file;
			uint16_t		chmod;
			uid_t			chown_uid;
			gid_t			chown_gid;
			std::string		username;
			std::string		password;
			bool			disabled;
			int				sockfd;

			// Methods
			void	initialize();
			int		start();
			void	close();

		private:

			// Methods
			std::string	validate(const std::string& key, ConfigParser::ConfigEntry *entry);
			std::string	expand_vars(std::map<std::string, std::string>& env, const std::string& key);
			bool		resolve_user(const std::string& value);
			bool		apply_ownership(uid_t uid, gid_t gid);


	};

#pragma endregion
