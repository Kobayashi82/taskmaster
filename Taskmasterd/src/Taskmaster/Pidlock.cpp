/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Pidlock.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 12:41:36 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/10 18:17:29 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Taskmaster/Pidlock.hpp"
	#include "Logging/TaskmasterLog.hpp"

	#include <fcntl.h>															// open()
	#include <unistd.h>															// close()
	#include <cstring>															// strerror()
	#include <sys/file.h>														// flock()

#pragma endregion

#pragma region "Constructors"

	Pidlock::Pidlock() : _pidfd(-1), _locked(false) {}

	Pidlock::~Pidlock() { unlock(); }

#pragma endregion

#pragma region "Is Locked"

	bool Pidlock::is_locked(const std::string& pidfile) const {
		int lockfd = open(pidfile.c_str(), O_RDWR);
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

	void Pidlock::unlock() {
		if (!_locked) return;
		if (_pidfd >= 0) ::close(_pidfd); _pidfd = -1;
		if (std::remove(_pidfile.c_str()) && errno != ENOENT)	Log.error("Pidlock: failed to unlock - " + std::string(strerror(errno)));
		else													Log.debug("Pidlock: unlocked at " + _pidfile);
		_locked = false;
	}

#pragma endregion

#pragma region "Lock"

	int Pidlock::lock(const std::string& pidfile) {
		if (_locked) unlock();

		_pidfile = pidfile;
		_pidfd = open(_pidfile.c_str(), O_RDWR|O_CREAT|O_TRUNC, 0640);
		if (_pidfd < 0) {
			Log.critical("Pidlock: failed to create lock - " + std::string(strerror(errno)));
			return (1);
		}

		if (flock(_pidfd, LOCK_EX|LOCK_NB) == -1) {
			Log.critical("Pidlock: lock already in use");
			if (_pidfd >= 0) ::close(_pidfd); _pidfd = -1;
			return (1);
		}

		std::string pid_str = std::to_string(getpid()) + "\n";
		if (write(_pidfd, pid_str.c_str(), pid_str.length()) == -1) {
			Log.error("Pidlock: failed to write PID - " + std::string(strerror(errno)));
		}

		_locked = true;
		Log.debug("Pidlock: locked at " + _pidfile);

		return (0);
	}

#pragma endregion
