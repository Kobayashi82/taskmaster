/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Process.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/02 17:23:05 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/11 14:00:32 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Taskmaster/Taskmaster.hpp"

#pragma endregion

#pragma region "Constructors"

	Process::Process() :
		pid(0),
		process_num(0),
		status(STOPPED),
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
		terminated(false),
		std_in(-1),
		std_out(-1),
		std_err(-1)
	{}

#pragma endregion
