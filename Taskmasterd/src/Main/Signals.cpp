/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Signals.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/16 11:44:57 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/09 20:08:07 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Config.hpp"
	#include "Logging/TaskmasterLog.hpp"
	#include "Main/Signals.hpp"

	#include <iostream>															// std::exit()
	#include <unistd.h>															// close()
	#include <csignal>															// std::signal()
	#include <sys/wait.h>														// waitpid()

#pragma endregion

	volatile sig_atomic_t Signals::signum = 0;

#pragma region "Handlers"

	#pragma region "SIGQUIT"

		void Signals::sigquit_handler(int sig) {
			Log.info("Signal: received SIGQUIT indicating exit request");
			signum = sig;
		}

	#pragma endregion

	#pragma region "SIGINT"

		void Signals::sigint_handler(int sig) {
			Log.info("Signal: received SIGINT indicating exit request");
			signum = sig;
		}

	#pragma endregion

	#pragma region "SIGTERM"

		void Signals::sigterm_handler(int sig) {
			Log.info("Signal: received SIGTERM indicating exit request");
			signum = sig;
		}

	#pragma endregion

	#pragma region "SIGHUP"

		void Signals::sighup_handler(int sig) { (void) sig;
			Log.info("Signal: SIGHUP received. Reloading configuration");
			Config.reload();
		}

	#pragma endregion

	#pragma region "SIGSEV"

		void Signals::sigsev_handler(int sig) {
			Log.critical("Signal: SIGSEV received. Segmentation fault");

			// Cleanup
			signal(SIGSEGV, SIG_DFL);
			raise(SIGSEGV);
			std::exit(128 + sig);
		}

	#pragma endregion

	#pragma region "SIGPIPE"

		void Signals::sigpipe_handler(int sig) { (void) sig;

		}

	#pragma endregion

	#pragma region "SIGCHLD"

		void Signals::sigchld_handler(int sig) { (void) sig;

		}

	#pragma endregion

#pragma endregion

#pragma region "Set"

	void Signals::set_for_load() {
		std::signal(SIGQUIT, sigquit_handler);
		std::signal(SIGINT, sigint_handler);
		std::signal(SIGTERM, sigterm_handler);
		std::signal(SIGHUP, SIG_IGN);
		std::signal(SIGSEGV, sigsev_handler);
		std::signal(SIGPIPE, SIG_IGN);
	}

#pragma endregion
