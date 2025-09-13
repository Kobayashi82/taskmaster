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

	#include "Taskmaster/Taskmaster.hpp"

	#include <csignal>															// std::signal()
	#include <unistd.h>															// close()
	#include <cstring>															// strerror()
	#include <iostream>															// std::exit()
	#include <sys/wait.h>														// waitpid()
	#include <sys/signalfd.h>													// struct signalfd_siginfo, signalfd()

#pragma endregion

#pragma region "Variables"

	int							Signal::signal_fd = -1;
	volatile sig_atomic_t		Signal::signum = 0;
	sigset_t					Signal::oldmask;
	std::vector<std::string>	Signal::signals = {
		"ZERO",			// 0
		"SIGHUP",		// 1
		"SIGINT",		// 2
		"SIGQUIT",		// 3
		"SIGILL",		// 4
		"SIGTRAP",		// 5
		"SIGABRT",		// 6
		"SIGBUS",		// 7
		"SIGFPE",		// 8
		"SIGKILL",		// 9
		"SIGUSR1",		// 10
		"SIGSEGV",		// 11
		"SIGUSR2",		// 12
		"SIGPIPE",		// 13
		"SIGALRM",		// 14
		"SIGTERM",		// 15
		"SIGSTKFLT",	// 16
		"SIGCHLD",		// 17
		"SIGCONT",		// 18
		"SIGSTOP",		// 19
		"SIGTSTP",		// 20
		"SIGTTIN",		// 21
		"SIGTTOU",		// 22
		"SIGURG",		// 23
		"SIGXCPU",		// 24
		"SIGXFSZ",		// 25
		"SIGVTALRM",	// 26
		"SIGPROF",		// 27
		"SIGWINCH",		// 28
		"SIGIO",		// 29
		"SIGPWR",		// 30
		"SIGSYS"		// 31
	};

#pragma endregion

#pragma region "Handlers"

	#pragma region "SIGQUIT"

		void Signal::sigquit_handler(int sig) {
			Log.info("Signal: received SIGQUIT indicating exit request");
			signum = sig;
			tskm.programs[2].stop(tskm.programs[2].processes[0]);
			// tskm.programs[1].start(tskm.programs[1].processes[0]);
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

		void Signal::sigabrt_handler(int sig) {
			Log.critical("Signal: SIGABRT received. Abort execution");

			signal(SIGABRT, SIG_DFL);
			tskm.cleanup();
			raise(SIGABRT);
			std::exit(128 + sig);
		}

	#pragma endregion

	#pragma region "SIGSEGV"

		void Signal::sigsegv_handler(int sig) {
			Log.critical("Signal: SIGSEGV received. Segmentation fault");

			signal(SIGSEGV, SIG_DFL);
			tskm.cleanup();
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
			pid_t	pid;
			int		status;

			while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
				auto it = tskm.processes.find(pid);
				if (it == tskm.processes.end()) continue;

				Process *proc = it->second;

				proc->terminated = true;

				if (WIFEXITED(status)) {
					proc->exit_code = WEXITSTATUS(status);
					proc->exit_reason = (std::find(proc->exitcodes.begin(), proc->exitcodes.end(), proc->exit_code) != proc->exitcodes.end()) ? "exited" : "unexpected";
				} else if (WIFSIGNALED(status)) {
					int sig_n = WTERMSIG(status);
					proc->exit_code = 128 + sig_n;
				    switch (sig_n) {
						case SIGQUIT: proc->exit_reason = "quit";					break;
						case SIGINT:  proc->exit_reason = "interrupted";			break;
						case SIGTERM: proc->exit_reason = "terminated";				break;
						case SIGHUP:  proc->exit_reason = "hangup";					break;
						case SIGABRT: proc->exit_reason = "aborted";				break;
						case SIGSEGV: proc->exit_reason = "segmentation fault";		break;
						case SIGPIPE: proc->exit_reason = "broken pipe";			break;
						case SIGKILL: proc->exit_reason = "killed";					break;
						default:      proc->exit_reason = "unknown";				break;
					}
				}

				// Si no hay datos, cerrar std_out y std_err
				// Si los hay, ya se hará en epoll
				// Si shutting down taskmaster, seguir ciclo normal (deberia de salir de epoll cuando termine de procesar todo... ya veremos como lo hago)

				if (proc->std_in != -1) {
					tskm.epoll.remove(proc->std_in);
					::close(proc->std_in);
					EventInfo *event = tskm.event.get(proc->std_in);
					for (auto& fd : event->in)	tskm.event.in_remove(event->fd, fd);
					tskm.event.remove(proc->std_in);
					proc->std_in = -1;
				}
			}
		}

	#pragma endregion

#pragma endregion

#pragma region "Set"

	void Signal::set_for_load() {
		std::signal(SIGQUIT, sigquit_handler);
		std::signal(SIGINT, sigint_handler);
		std::signal(SIGTERM, sigterm_handler);
		std::signal(SIGHUP, SIG_IGN);
		std::signal(SIGABRT, sigabrt_handler);
		std::signal(SIGSEGV, sigsegv_handler);
		std::signal(SIGPIPE, SIG_IGN);
		std::signal(SIGCHLD, SIG_IGN);
	}

	void Signal::set_default() {
		std::signal(SIGQUIT, SIG_DFL);
		std::signal(SIGINT, SIG_DFL);
		std::signal(SIGTERM, SIG_DFL);
		std::signal(SIGHUP, SIG_DFL);
		std::signal(SIGABRT, SIG_DFL);
		std::signal(SIGSEGV, SIG_DFL);
		std::signal(SIGPIPE, SIG_DFL);
		std::signal(SIGCHLD, SIG_DFL);
	}

#pragma endregion>

#pragma region "Create FD"

	// Warning: calling create() twice will close the previous fd, but it may still be registered in epoll
	int Signal::create() {
		if (signal_fd >= 0) close();

		set_default();

		sigset_t mask;
		sigemptyset(&mask);
		sigaddset(&mask, SIGQUIT);
		sigaddset(&mask, SIGINT);
		sigaddset(&mask, SIGTERM);
		sigaddset(&mask, SIGHUP);
		sigaddset(&mask, SIGABRT);
		sigaddset(&mask, SIGSEGV);
		sigaddset(&mask, SIGPIPE);
		sigaddset(&mask, SIGCHLD);

		if (pthread_sigmask(SIG_BLOCK, &mask, &oldmask)) {
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

		if (pthread_sigmask(SIG_SETMASK, &oldmask, nullptr)) {
			Log.error("Signal: failed to restore old signal mask - " + std::string(strerror(errno)));
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
				case SIGQUIT:	sigquit_handler(fdsi.ssi_signo);	break; // return (1);
				case SIGINT:	sigint_handler(fdsi.ssi_signo);		return (1);
				case SIGTERM:	sigterm_handler(fdsi.ssi_signo);	return (1);
				case SIGHUP:	sighup_handler(fdsi.ssi_signo);		break;
				case SIGABRT:	sigabrt_handler(fdsi.ssi_signo);	return (1);
				case SIGSEGV:	sigsegv_handler(fdsi.ssi_signo);	return (1);
				case SIGPIPE:	sigpipe_handler(fdsi.ssi_signo);	break;
				case SIGCHLD:	sigchld_handler(fdsi.ssi_signo);	break;
			}
		}

		return (0);
	}

#pragma endregion
