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
	#include "Loop/Signal.hpp"

	#include <csignal>															// std::signal()
	#include <unistd.h>															// close()
	#include <cstring>															// strerror()
	#include <iostream>															// std::exit()
	#include <sys/wait.h>														// waitpid()
	#include <sys/signalfd.h>													// struct signalfd_siginfo, signalfd()

#pragma endregion

#pragma region "Variables"

	int						Signal::signal_fd = -1;
	volatile sig_atomic_t	Signal::signum = 0;

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

	#pragma region "SIGSEGV"

		void Signal::sigsegv_handler(int sig) {
			Log.critical("Signal: SIGSEGV received. Segmentation fault");

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
		std::signal(SIGSEGV, sigsegv_handler);
		std::signal(SIGPIPE, SIG_IGN);
		std::signal(SIGCHLD, SIG_IGN);
	}

	void Signal::set_default() {
		std::signal(SIGQUIT, SIG_DFL);
		std::signal(SIGINT, SIG_DFL);
		std::signal(SIGTERM, SIG_DFL);
		std::signal(SIGHUP, SIG_DFL);
		std::signal(SIGSEGV, SIG_DFL);
		std::signal(SIGPIPE, SIG_DFL);
		std::signal(SIGCHLD, SIG_DFL);
	}

#pragma endregion>

#pragma region "Create FD"

	// Warning: calling create() twice will close the previous fd, but it may still be registered in epoll
	int Signal::create() {
		if (signal_fd >= 0) close();

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
			Log.critical("Signal: failed to block signals - " + std::string(strerror(errno)));
			return (-1);
		}

		signal_fd = signalfd(-1, &mask, SFD_CLOEXEC | SFD_NONBLOCK);
		if (signal_fd == -1) {
			Log.critical("Signal: failed to create signal FD - " + std::string(strerror(errno)));
			return (-1);
		}

		Log.debug("Signal: signal FD created: " + std::to_string(signal_fd));

		return (signal_fd);
	}

#pragma endregion

#pragma region "Close FD"

	// Warning: calling close() will close the fd, but it may still be registered in epoll
	void Signal::close() {
		if (signal_fd >= 0) {
			if (::close(signal_fd) == -1) Log.error("Signal: failed to close signal FD " + std::to_string(signal_fd));
			signal_fd = -1;
		}
	}

#pragma endregion

#pragma region "Process"

	int Signal::process() {
		struct signalfd_siginfo fdsi;
		ssize_t bytes_read;

		while (true) {
			bytes_read = read(signal_fd, &fdsi, sizeof(fdsi));
			if (bytes_read == 0) {
				Log.error("Signal: signal FD closed unexpectedly");
				break;
			}
			else if (bytes_read == -1) {
				if (errno == EAGAIN || errno == EWOULDBLOCK) break;		// no data
				if (errno == EINTR) continue;							// interrupted, retry
				Log.error("Signal: failed to receive signal - " + std::string(strerror(errno)));
				break;
			}
			else if (bytes_read != sizeof(fdsi)) {
				Log.error("Signal: corrupted signal received");
				break;
			}

			switch (fdsi.ssi_signo) {
				case SIGQUIT:	sigquit_handler(fdsi.ssi_signo);	return (1);
				case SIGINT:	sigint_handler(fdsi.ssi_signo);		return (1);
				case SIGTERM:	sigterm_handler(fdsi.ssi_signo);	return (1);
				case SIGHUP:	sighup_handler(fdsi.ssi_signo);		break;
				case SIGSEGV:	sigsegv_handler(fdsi.ssi_signo);	return (1);
				case SIGPIPE:	sigpipe_handler(fdsi.ssi_signo);	break;
				case SIGCHLD:	sigchld_handler(fdsi.ssi_signo);	break;
			}
		}

		return (0);
	}

#pragma endregion
