/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Signals.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 18:09:55 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/09 19:51:32 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include <csignal>															// sig_atomic_t

#pragma endregion

#pragma region "Signals"

	class Signals {

		private:

			static void	sigquit_handler(int sig);
			static void	sigint_handler(int sig);
			static void	sigterm_handler(int sig);
			static void	sighup_handler(int sig);
			static void	sigsev_handler(int sig);
			static void	sigpipe_handler(int sig);
			static void	sigchld_handler(int sig);

		public:

			// Constructors
			Signals() = delete;
			Signals(const Signals&) = delete;
			Signals(Signals&&) = delete;
			~Signals() = delete;

			// Overloads
			Signals& operator=(const Signals&) = delete;
			Signals& operator=(Signals&&) = delete;

			static volatile sig_atomic_t signum;

			static void set_for_load();

	};

#pragma endregion
