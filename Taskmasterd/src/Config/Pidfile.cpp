/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Pidfile.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 12:41:36 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/09 14:55:50 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Pidfile.hpp"
	#include "Logging/TaskmasterLog.hpp"

	#include <fcntl.h>															// open()
	#include <unistd.h>															// close()
	#include <cstring>															// strerror()
	#include <sys/file.h>														// flock()

#pragma endregion

#pragma region "Constructors"

	Pidfile::Pidfile(const std::string& pidfile) : _pidfile(pidfile), _pidfd(-1), locked(false) {}

	Pidfile::~Pidfile() { unlock(); }

#pragma endregion

#pragma region "Is Locked"

	bool Pidfile::is_locked() const {
		int lockfd = open(_pidfile.c_str(), O_RDWR);
		if (lockfd < 0) return (errno != ENOENT);

		if (flock(lockfd, LOCK_EX|LOCK_NB) == -1) {
			::close(lockfd);
			return (true);
		}
		
		::close(lockfd);
		return (false);
	}

#pragma endregion

#pragma region "Unlock"

	void Pidfile::unlock() {
		if (!locked) return;
		if (_pidfd >= 0) ::close(_pidfd); _pidfd = -1;
		if (std::remove(_pidfile.c_str()) && errno != ENOENT)	Log.error("Pidfile: failed to unlock - " + std::string(strerror(errno)));
		else													Log.debug("Pidfile: unlock");
		locked = false;
	}

#pragma endregion

#pragma region "Lock"

	int Pidfile::lock() {
		_pidfd = open(_pidfile.c_str(), O_RDWR|O_CREAT|O_TRUNC, 0640);
		if (_pidfd < 0) {
			Log.critical("Pidfile: failed to create lock - " + std::string(strerror(errno)));
			return (1);
		}

		if (flock(_pidfd, LOCK_EX|LOCK_NB) == -1) {
			Log.critical("Pidfile: lock already in use");
			if (_pidfd >= 0) ::close(_pidfd); _pidfd = -1;
			return (1);
		}

		std::string pid_str = std::to_string(getpid()) + "\n";
		if (write(_pidfd, pid_str.c_str(), pid_str.length()) == -1) {
			Log.error("Pidfile: failed to write PID - " + std::string(strerror(errno)));
		}

		locked = true;
		Log.debug("Pidfile: locked");

		return (0);
	}

#pragma endregion
