/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Process.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/02 17:23:05 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/03 18:43:37 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Programs/Process.hpp"

#pragma endregion

#pragma region "Constructors"

	Process::Process() {
		pid = 0;
		status = STOPPED;
		start_time = -1;
		stop_time = -1;
		change_time = -1;
		uptime = -1;
		restart_count = 0;
		killwait_secs = 0;
		exit_code = 0;
		exit_reason = "";
		spawn_error = "";
	}

#pragma endregion
