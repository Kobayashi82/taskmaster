/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Epoll.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/18 11:54:41 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/10 13:04:53 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include <cstdint>															// uint16_t

#pragma endregion

#pragma region "Epoll"

	class Epoll {

		private:

			//	Variables
			int							_epoll_fd;
			static constexpr uint16_t	MAX_EVENTS = 10;

		public:

			// Constructors
			Epoll();
			Epoll(const Epoll&) = delete;
			Epoll(Epoll&&) = delete;
			~Epoll();

			// Overloads
			Epoll& operator=(const Epoll&) = delete;
			Epoll& operator=(Epoll&&) = delete;

			//	Methods
			int		create();
			void	close();
			int		remove(int fd);
			int		set(int fd, bool epollin, bool epollout);
			int		add(int fd, bool epollin, bool epollout);
			int		wait();

	};

#pragma endregion
