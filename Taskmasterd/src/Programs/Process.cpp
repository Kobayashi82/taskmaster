/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Process.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/02 17:23:05 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/12 16:35:04 by vzurera-         ###   ########.fr       */
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
		pid(0),
		process_num(0),
		status(ProcessState::STOPPED),
		start_time(0),
		stop_time(0),
		change_time(0),
		uptime(0),
		restart_count(0),
		killwait_secs(0),
		exit_code(0),
		exit_reason(""),
		spawn_error(""),
		program_name(""),
		started_once(false),
		manual_stopped(false),
		terminated(false),
		std_in(-1),
		std_out(-1),
		std_err(-1)
	{}

#pragma endregion

#pragma region "History"

	#pragma region "Add"

		void Process::history_add() {
			if (history.size() > MAX_HISTORY_SIZE) history.pop_front();

			history.emplace_back(name, program_name, status, exit_code);
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
