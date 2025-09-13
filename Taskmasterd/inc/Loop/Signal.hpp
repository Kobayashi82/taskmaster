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

		public:

			// Constructors
			Signal() = delete;
			Signal(const Signal&) = delete;
			Signal(Signal&&) = delete;
			~Signal() = delete;

			// Overloads
			Signal& operator=(const Signal&) = delete;
			Signal& operator=(Signal&&) = delete;

			//Variables
			static int						signal_fd;
			static volatile sig_atomic_t	signum;
			static sigset_t					oldmask;
			static std::vector<std::string>	signals;

			// Handlers
			static void	sigquit_handler(int sig);
			static void	sigint_handler(int sig);
			static void	sigterm_handler(int sig);
			static void	sighup_handler(int sig);
			static void	sigabrt_handler(int sig);
			static void	sigsegv_handler(int sig);
			static void	sigpipe_handler(int sig);
			static void	sigchld_handler(int sig);

			// Methods
			static void	set_for_load();
			static void	set_default();
			static int	create();
			static void	close();
			static int	process();

	};

#pragma endregion
