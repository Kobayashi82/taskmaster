/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Daemon.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 13:53:12 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/09 21:27:37 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Main/Taskmasterd.hpp"

	#include <unistd.h>															// fork(), setsid(), chdir(), close(), getpid()
	#include <sys/stat.h>														// umask()
	#include <sstream>															// std::ostringstream
	#include <iomanip>															// std::setw(), std::setfill()
	#include <cstring>															// strerror()

#pragma endregion

#pragma region "Daemonize"

	int daemonize(Pidfile& pidfile) {

		if (!TaskMaster.nodaemon) {
			// 1. fork()
			int pid = fork();
			if (pid < 0) {
				Log.critical("Daemon: first fork failed - " + std::string(strerror(errno)));
				return (1);
			}
			if (pid > 0) return (-1);
			Log.debug("Daemon: first fork completed");

			// 2. setsid()
			if (setsid() < 0) {
				Log.critical("Daemon: setsid() failed - " + std::string(strerror(errno)));
				return (1);
			}
			Log.debug("Daemon: setsid completed");

			// 3. fork()
			pid = fork();
			if (pid < 0) {
				Log.critical("Daemon: second fork failed - " + std::string(strerror(errno)));
				return (1);
			}
			if (pid > 0) return (-1);
			Log.debug("Daemon: second fork completed");

			// 4. close()
			close(0); close(1); close(2);
			Log.debug("Daemon: standard file descriptors closed");

			Log.debug("Daemon: completed successfully");
		}

		// 5. umask()
		umask(TaskMaster.umask);
		std::ostringstream oss;
		oss << "Taskmasterd: umask set to " << std::oct << std::setw(3) << std::setfill('0') << TaskMaster.umask;
		Log.debug(oss.str());

		// 6. chdir()
		if (chdir("/"))					Log.warning("Taskmasterd: failed to change directory to / - " + std::string(strerror(errno)));
		else							Log.debug("Taskmasterd: working directory set to /");

		// 7. flock()
		if (pidfile.lock()) return (1);

		TaskMaster.pid = getpid();

		return (0);
	}

#pragma endregion
