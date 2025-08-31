/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Daemon.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/14 23:07:15 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/31 13:33:33 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Options.hpp"
	#include "Logging/Logging.hpp"
	#include "Config/Signals.hpp"

	#include "Network/Client.hpp"

	#include <iostream>															// std::cerr(), std::exit()
	#include <fcntl.h>															// open()
	#include <unistd.h>															// fork(), setsid(), chdir(), close()
	#include <sys/stat.h>														// umask()
	#include <sys/file.h>														// flock()

#pragma endregion

#pragma region "Daemonize"

	int daemonize() {
		// 1. fork()
		int pid = fork();
		if (pid < 0) {
			Log->critical("Daemon: First fork() failed");
			std::cerr << "Daemon: First fork() failed\n";
			return (1);
		}
		if (pid > 0) std::exit(0);
		Log->debug("Daemon: First fork() completed");

		// 2. setsid()
		if (setsid() < 0) { Log->critical("Daemon: setuid() failed"); std::exit(1); }
		Log->debug("Daemon: setsid() completed");

		// 3. fork()
		pid = fork();
		if (pid < 0) { Log->critical("Daemon: Second fork() failed"); std::exit(1); }
		if (pid > 0) std::exit(0);
		Log->debug("Daemon: Second fork() completed");

		// 4. signal()
		if (!signal_set())	Log->debug("Daemon: All signal handlers successfully installed");
		else				Log->debug("Daemon: Failed to register one or more signal handlers");

		// 5. umask()
		umask(022);
		Log->debug("Daemon: umask() set");

		// 6. chdir()
		if (chdir("/"))	Log->warning("Daemon: chdir() failed");
		else			Log->debug("Daemon: Working directory changed");

		// 7. close()
		close(0); close(1); close(2);
		Log->debug("Daemon: Standard file descriptors closed");

		// 8. flock()
		Options::lockfd = open("/var/lock/matt_daemon.lock", O_RDWR|O_CREAT|O_TRUNC, 0640);
		if (Options::lockfd < 0 || flock(Options::lockfd, LOCK_EX|LOCK_NB)) {
			if (Options::lockfd >= 0) close(Options::lockfd);
			Log->critical("Daemon: Already running");
			std::exit(1);
		}
		Log->debug("Daemon: Lock set");

		return (0);
	}

#pragma endregion
