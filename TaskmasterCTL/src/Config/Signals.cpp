/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Signals.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/16 11:44:57 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/20 14:05:42 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Options.hpp"
	#include "Config/Logging.hpp"
	#include "Config/Signals.hpp"

	#include "Network/Client.hpp"
	#include "Network/Epoll.hpp"

	#include <iostream>															// std::getline(), std::cerr(), std::exit()
	#include <unistd.h>															// close()
	#include <csignal>															// std::signal()
	#include <sys/wait.h>														// waitpid()

#pragma endregion

#pragma region "Handlers"

	#pragma region "SIGINT"

		static void sigint_handler(int sig) {
			Log->info("Signal: SIGINT received. Closing Daemon");
			Options::signum = sig;
			Epoll::Running = false;
		}

	#pragma endregion

	#pragma region "SIGTERM"

		static void sigterm_handler(int sig) {
			Log->info("Signal: SIGTERM received. Closing Daemon");
			Options::signum = sig;
			Epoll::Running = false;
		}

	#pragma endregion

	#pragma region "SIGHUP"

		static void sighup_handler(int sig) {
			Log->info("Signal: SIGHUP received. No reload configuration required");
			Options::signum = sig;
			Epoll::Running = false;
		}

	#pragma endregion

	#pragma region "SIGQUIT"

		static void sigquit_handler(int sig) {
			Log->info("Signal: SIGQUIT received. Closing Daemon");
			Options::signum = sig;
			Epoll::Running = false;
		}

	#pragma endregion

	#pragma region "SIGSEV"

		static void sigsev_handler(int sig) {
			Log->critical("Signal: SIGSEV received. Segmentation fault");
			
			for (auto& client_pair : clients) {
				Client *client = client_pair.second.get();
				if (client && client->shell_running && client->shell_pid) kill(client->shell_pid, SIGKILL);
			}
			Epoll::close();
			if (Options::lockfd >= 0) close(Options::lockfd);
			if (Options::sockfd >= 0) close(Options::sockfd);
			unlink("/var/lock/matt_daemon.lock");

			signal(SIGSEGV, SIG_DFL);
			raise(SIGSEGV);
			std::exit(128 + sig);
		}

	#pragma endregion

	#pragma region "SIGPIPE"

		static void sigpipe_handler(int sig) { (void) sig;
			Log->info("Signal: SIGPIPE received. Shell or client connection closed");
		}

	#pragma endregion

	#pragma region "SIGCHLD"

		static void sigchld_handler(int sig) { (void) sig;
			int pid, status;

			while ((pid = waitpid(-1, &status, WNOHANG)) > 0) terminated_pids.push_back(pid);
		}

	#pragma endregion

#pragma endregion

#pragma region "Set"

	int signal_set() {
		int result = 0;

		if (std::signal(SIGINT,  sigint_handler)	== SIG_ERR) { result++; Log->warning("Signal: SIGINT failed");  }	// Interrupt from keyboard (Ctrl+C)
		if (std::signal(SIGTERM, sigterm_handler)	== SIG_ERR) { result++; Log->warning("Signal: SIGTERM failed"); }	// Request to terminate the program gracefully (sent by 'kill' or system shutdown)
		if (std::signal(SIGHUP,  sighup_handler)	== SIG_ERR) { result++; Log->warning("Signal: SIGHUP failed");  }	// Terminal hangup or controlling process terminated (often used to reload config)
		if (std::signal(SIGQUIT, sigquit_handler)	== SIG_ERR) { result++; Log->warning("Signal: SIGQUIT failed"); }	// Quit from keyboard (Ctrl+\)
		if (std::signal(SIGPIPE, sigpipe_handler)	== SIG_ERR) { result++; Log->warning("Signal: SIGPIPE failed"); }	// Broken pipe (write to pipe with no readers)
		if (std::signal(SIGSEGV, sigsev_handler)	== SIG_ERR) { result++; Log->warning("Signal: SIGSEGV failed"); }	// Invalid memory reference (segmentation fault)
		if (std::signal(SIGCHLD, sigchld_handler)	== SIG_ERR) { result++; Log->warning("Signal: SIGCHLD failed"); }	// Child process stopped or terminated (used to reap zombies)

		return (result);
	}

#pragma endregion
