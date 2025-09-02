/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Signals.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/16 11:44:57 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/20 20:33:53 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Options.hpp"
	#include "Config/Signals.hpp"

	#include <iostream>															// std::getline(), std::cerr(), std::exit()
	#include <unistd.h>															// close()
	#include <csignal>															// std::signal()

#pragma endregion

#pragma region "Handlers"

	#pragma region "SIGINT"

		static void sigint_handler(int sig) {
			// Log->info("Signal: SIGINT received. Closing Daemon");
			Options::signum = sig;
			// // Epoll::Running = false;
		}

	#pragma endregion

	#pragma region "SIGTERM"

		static void sigterm_handler(int sig) {
			// Log->info("Signal: SIGTERM received. Closing Daemon");
			Options::signum = sig;
			// Epoll::Running = false;
		}

	#pragma endregion

	#pragma region "SIGHUP"

		static void sighup_handler(int sig) {
			// Log->info("Signal: SIGHUP received. No reload configuration required");
			Options::signum = sig;
			// Epoll::Running = false;
		}

	#pragma endregion

	#pragma region "SIGQUIT"

		static void sigquit_handler(int sig) {
			// Log->info("Signal: SIGQUIT received. Closing Daemon");
			Options::signum = sig;
			// Epoll::Running = false;
		}

	#pragma endregion

	#pragma region "SIGSEV"

		static void sigsev_handler(int sig) {
			// Log->critical("Signal: SIGSEV received. Segmentation fault");

			// Epoll::close();
			if (Options::sockfd >= 0) close(Options::sockfd);

			signal(SIGSEGV, SIG_DFL);
			raise(SIGSEGV);
			std::exit(128 + sig);
		}

	#pragma endregion

	#pragma region "SIGPIPE"

		static void sigpipe_handler(int sig) { (void) sig;
			// Log->info("Signal: SIGPIPE received. Shell or client connection closed");
		}

	#pragma endregion

#pragma endregion

#pragma region "Set"

	int signal_set() {
		int result = 0;

		if (std::signal(SIGINT,  sigint_handler)	== SIG_ERR) result++;	// Interrupt from keyboard (Ctrl+C)
		if (std::signal(SIGTERM, sigterm_handler)	== SIG_ERR) result++;	// Request to terminate the program gracefully (sent by 'kill' or system shutdown)
		if (std::signal(SIGHUP,  sighup_handler)	== SIG_ERR) result++;	// Terminal hangup or controlling process terminated (often used to reload config)
		if (std::signal(SIGQUIT, sigquit_handler)	== SIG_ERR) result++;	// Quit from keyboard (Ctrl+\)
		if (std::signal(SIGPIPE, sigpipe_handler)	== SIG_ERR) result++;	// Broken pipe (write to pipe with no readers)
		if (std::signal(SIGSEGV, sigsev_handler)	== SIG_ERR) result++;	// Invalid memory reference (segmentation fault)

		return (result);
	}

#pragma endregion
