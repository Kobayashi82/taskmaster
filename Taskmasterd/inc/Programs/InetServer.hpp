/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   InetServer.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/05 19:26:45 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/06 22:05:18 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include <cstdint>															// uint16_t
	#include <string>															// std::string
	#include <map>																// std::map
	#include <vector>															// std::vector

#pragma endregion

#pragma region "InetServer"

	class InetServer {

		public:

			// Constructors
			InetServer() = default;
			InetServer(const InetServer&) = default;
			InetServer(InetServer&&) = default;
			~InetServer();

			// Overloads
			InetServer& operator=(const InetServer&) = default;
			InetServer& operator=(InetServer&&) = default;

			// Variables
			std::string		section;
			std::string		url;
			std::string		host;
			std::string		hostname;
			uint16_t		port;
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
			std::string	resolve_host(const std::string& host);

	};

#pragma endregion
