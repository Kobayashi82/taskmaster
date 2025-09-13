/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Process.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/02 17:23:05 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/13 13:29:02 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Taskmaster/Taskmaster.hpp"

	#include <iomanip>															// std::put_time()
	#include <sstream>															// std::ostringstream

#pragma endregion

#pragma region "Status Event"

	std::string StatusEvent::to_string() const {
		std::ostringstream oss;
		oss << Utils::get_timestamp() << " " << tskm.status_names[static_cast<int>(status)];
		if (status == ProcessState::STOPPED || status == ProcessState::BACKOFF || status == ProcessState::EXITED || status == ProcessState::FATAL) {
			if (exit_code > 128 && exit_code < 160)	oss << " with signal: " << Signal::signals[exit_code - 128] << " (" << (exit_code - 128) << ")";
			else									oss << " with exit code: " << exit_code;
		}

		return (oss.str());
	}

#pragma endregion

#pragma region "Constructors"

	Process::Process() :
		program_name(""),
		process_num(0),

		pid(0),
		status(ProcessState::STOPPED),
		started_once(false),
		stopped_manual(false),
		terminated(false),
		killed(false),
		start_time(0),
		stop_time(0),
		change_time(0),
		exit_code(0),
		exit_reason(""),
		restart_count(0),
		killwaitsecs(3),

		std_in(-1),
		std_out(-1),
		std_err(-1)
	{}

#pragma endregion

#pragma region "History"

	#pragma region "Add"

		void Process::history_add() {
			if (history.size() > MAX_HISTORY_SIZE) history.pop_front();

			static const std::vector<std::string> status_names = {
				"STOPPED",		// 0
				"STARTING",		// 1
				"RUNNING",		// 2
				"BACKOFF",		// 3
				"STOPPING",		// 4
				"EXITED",		// 5
				"FATAL",		// 6
				"UNKNOWN"		// 7
			};

			history.emplace_back(name, program_name, status, exit_code);
			Log.debug("Process: " + name + " changed state to " + status_names[static_cast<int>(status)]);
		}

	#pragma endregion

	#pragma region "Get"

		std::string Process::history_get(uint16_t tail) {
		    std::ostringstream oss;
			size_t n = history.size();

			if (tail > n) tail = n;

			for (size_t i = n - tail; i < n; ++i)
				oss << history[i].to_string() << "\n";

			return (oss.str());
		}

	#pragma endregion

	#pragma region "Clear"

		void Process::history_clear() {
			history.clear();
		}

	#pragma endregion

#pragma endregion
