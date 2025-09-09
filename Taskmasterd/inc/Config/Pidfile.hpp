/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Pidfile.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 12:43:00 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/09 13:46:11 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include <string>															// std::string

#pragma endregion

#pragma region "TaskManager"

	class Pidfile {

		private:

			// Variables
			std::string	_pidfile;
			int			_pidfd;
			bool		locked;

		public:

			// Constructors
			Pidfile() = delete;
			explicit Pidfile(const std::string& pidfile);
			Pidfile(const Pidfile&) = delete;
			Pidfile(Pidfile&&) = delete;
			~Pidfile();

			// Overloads
			Pidfile& operator=(const Pidfile&) = delete;
			Pidfile& operator=(Pidfile&&) = delete;

			// Methods
			bool	is_locked() const;
			void	unlock();
			int		lock();

	};

#pragma endregion
