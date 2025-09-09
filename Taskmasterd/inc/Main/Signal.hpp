/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Signal.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 18:09:55 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/09 22:04:11 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include <csignal>															// sig_atomic_t

#pragma endregion

#pragma region "Signal"

	class Signal {

		private:

			//Variables
			static int	sigfd;

			static void	sigquit_handler(int sig);
			static void	sigint_handler(int sig);
			static void	sigterm_handler(int sig);
			static void	sighup_handler(int sig);
			static void	sigsev_handler(int sig);
			static void	sigpipe_handler(int sig);
			static void	sigchld_handler(int sig);

		public:

			// Constructors
			Signal() = delete;
			Signal(const Signal&) = delete;
			Signal(Signal&&) = delete;
			~Signal() = delete;

			// Overloads
			Signal& operator=(const Signal&) = delete;
			Signal& operator=(Signal&&) = delete;

			static volatile sig_atomic_t signum;

			static void	set_for_load();
			static void	set_default();
			static void	set_ignore();

			static int	create_fd();
			static void	close_fd();

	};

#pragma endregion
