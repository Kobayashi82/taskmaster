/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Pidlock.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 12:43:00 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/13 17:54:04 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include <string>															// std::string

#pragma endregion

#pragma region "Pidlock"

	class Pidlock {

		private:

			// Variables
			int			_pidfd;
			bool		_locked;
			std::string	_pidfile;

		public:

			// Constructors
			Pidlock();
			Pidlock(const Pidlock&) = delete;
			Pidlock(Pidlock&&) = delete;
			~Pidlock();

			// Overloads
			Pidlock& operator=(const Pidlock&) = delete;
			Pidlock& operator=(Pidlock&&) = delete;

			// Methods
			void	close();
			bool	is_locked(const std::string& pidfile) const;
			void	unlock();
			int		lock(const std::string& pidfile);

	};

#pragma endregion
