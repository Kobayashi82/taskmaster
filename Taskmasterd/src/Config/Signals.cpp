/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Signals.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/16 11:44:57 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/09 18:37:22 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Signals.hpp"
	#include "Logging/TaskmasterLog.hpp"

	#include <iostream>															// std::getline(), std::cerr(), std::exit()
	#include <unistd.h>															// close()
	#include <csignal>															// std::signal()
	#include <sys/wait.h>														// waitpid()

#pragma endregion

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

		void Signals::sighup_handler(int sig) {
			Log.info("Signal: SIGHUP received. eload configuration required");
			signum = sig;
		}

	#pragma endregion

	#pragma region "SIGSEV"

		void Signals::sigsev_handler(int sig) {
			Log.critical("Signal: SIGSEV received. Segmentation fault");

			signal(SIGSEGV, SIG_DFL);
			raise(SIGSEGV);
			std::exit(128 + sig);
		}

	#pragma endregion

	#pragma region "SIGPIPE"

		void Signals::sigpipe_handler(int sig) { signum = sig;
			Log.info("Signal: SIGPIPE received. Shell or client connection closed");
		}

	#pragma endregion

	#pragma region "SIGCHLD"

		void Signals::sigchld_handler(int sig) { signum = sig;
		}

	#pragma endregion

#pragma endregion

#pragma region "Set"

	int Signals::set() {
		int result = 0;

		if (std::signal(SIGINT,  sigint_handler)	== SIG_ERR) { result++; } // Log.warning("Signal: SIGINT failed");  }	// Interrupt from keyboard (Ctrl+C)
		if (std::signal(SIGTERM, sigterm_handler)	== SIG_ERR) { result++; } // Log.warning("Signal: SIGTERM failed"); }	// Request to terminate the program gracefully (sent by 'kill' or system shutdown)
		if (std::signal(SIGHUP,  sighup_handler)	== SIG_ERR) { result++; } // Log.warning("Signal: SIGHUP failed");  }	// Terminal hangup or controlling process terminated (often used to reload config)
		if (std::signal(SIGQUIT, sigquit_handler)	== SIG_ERR) { result++; } // Log.warning("Signal: SIGQUIT failed"); }	// Quit from keyboard (Ctrl+\)
		if (std::signal(SIGPIPE, sigpipe_handler)	== SIG_ERR) { result++; } // Log.warning("Signal: SIGPIPE failed"); }	// Broken pipe (write to pipe with no readers)
		if (std::signal(SIGSEGV, sigsev_handler)	== SIG_ERR) { result++; } // Log.warning("Signal: SIGSEGV failed"); }	// Invalid memory reference (segmentation fault)
		if (std::signal(SIGCHLD, sigchld_handler)	== SIG_ERR) { result++; } // Log.warning("Signal: SIGCHLD failed"); }	// Child process stopped or terminated (used to reap zombies)

		return (result);
	}

#pragma endregion
