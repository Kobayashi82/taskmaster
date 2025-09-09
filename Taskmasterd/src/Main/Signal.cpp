/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Signal.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/16 11:44:57 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/09 22:06:12 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Config.hpp"
	#include "Logging/TaskmasterLog.hpp"
	#include "Main/Signal.hpp"

	#include <iostream>															// std::exit()
	#include <unistd.h>															// close()
	#include <csignal>															// std::signal()
	#include <sys/signalfd.h>
	#include <fcntl.h>
	#include <sys/wait.h>														// waitpid()

#pragma endregion

#pragma region "Variables"

	volatile sig_atomic_t	Signal::signum = 0;
	int						Signal::sigfd = -1;

#pragma endregion

#pragma region "Handlers"

	#pragma region "SIGQUIT"

		void Signal::sigquit_handler(int sig) {
			Log.info("Signal: received SIGQUIT indicating exit request");
			signum = sig;
		}

	#pragma endregion

	#pragma region "SIGINT"

		void Signal::sigint_handler(int sig) {
			Log.info("Signal: received SIGINT indicating exit request");
			signum = sig;
		}

	#pragma endregion

	#pragma region "SIGTERM"

		void Signal::sigterm_handler(int sig) {
			Log.info("Signal: received SIGTERM indicating exit request");
			signum = sig;
		}

	#pragma endregion

	#pragma region "SIGHUP"

		void Signal::sighup_handler(int sig) { (void) sig;
			Log.info("Signal: SIGHUP received. Reloading configuration");
			Config.reload();
		}

	#pragma endregion

	#pragma region "SIGSEV"

		void Signal::sigsev_handler(int sig) {
			Log.critical("Signal: SIGSEV received. Segmentation fault");

			// Cleanup
			signal(SIGSEGV, SIG_DFL);
			raise(SIGSEGV);
			std::exit(128 + sig);
		}

	#pragma endregion

	#pragma region "SIGPIPE"

		void Signal::sigpipe_handler(int sig) { (void) sig;

		}

	#pragma endregion

	#pragma region "SIGCHLD"

		void Signal::sigchld_handler(int sig) { (void) sig;

		}

	#pragma endregion

#pragma endregion

#pragma region "Set"

	void Signal::set_for_load() {
		std::signal(SIGQUIT, sigquit_handler);
		std::signal(SIGINT, sigint_handler);
		std::signal(SIGTERM, sigterm_handler);
		std::signal(SIGHUP, SIG_IGN);
		std::signal(SIGSEGV, sigsev_handler);
		std::signal(SIGPIPE, SIG_IGN);
	}

	void Signal::set_default() {
		std::signal(SIGQUIT, SIG_DFL);
		std::signal(SIGINT, SIG_DFL);
		std::signal(SIGTERM, SIG_DFL);
		std::signal(SIGHUP, SIG_DFL);
		std::signal(SIGSEGV, SIG_DFL);
		std::signal(SIGPIPE, SIG_DFL);
	}

	void Signal::set_ignore() {
		std::signal(SIGQUIT, SIG_IGN);
		std::signal(SIGINT, SIG_IGN);
		std::signal(SIGTERM, SIG_IGN);
		std::signal(SIGHUP, SIG_IGN);
		std::signal(SIGSEGV, SIG_IGN);
		std::signal(SIGPIPE, SIG_IGN);
	}

#pragma endregion>

#pragma region "Create FD"

	int Signal::create_fd() {

		sigset_t mask;
		sigemptyset(&mask);
		sigaddset(&mask, SIGQUIT);
		sigaddset(&mask, SIGINT);
		sigaddset(&mask, SIGTERM);
		sigaddset(&mask, SIGHUP);
		sigaddset(&mask, SIGSEGV);
		sigaddset(&mask, SIGPIPE);
		sigaddset(&mask, SIGCHLD);

		if (pthread_sigmask(SIG_BLOCK, &mask, nullptr)) {
			perror("pthread_sigmask");
			return (-1);
		}

		sigfd = signalfd(-1, &mask, SFD_CLOEXEC | SFD_NONBLOCK);
		if (sigfd == -1) {
			perror("signalfd");
			return (-1);
		}

		Log.debug("Signal: signalfd created");

		return (sigfd);
	}

#pragma endregion

#pragma region "Close FD"

	void Signal::close_fd() {
		if (sigfd >= 0) close(sigfd);
		sigfd = -1;
	}

#pragma endregion

    // struct signalfd_siginfo fdsi;
    // ssize_t s;
    // while (true) {
    //     s = read(signalfd, &fdsi, sizeof(fdsi));
    //     if (s != sizeof(fdsi)) continue;

    //     std::cout << "Received signal: " << fdsi.ssi_signo << "\n";
    //     if (fdsi.ssi_signo == SIGTERM || fdsi.ssi_signo == SIGINT) {
    //         std::cout << "Exiting...\n";
    //         break;
    //     }
    // }
