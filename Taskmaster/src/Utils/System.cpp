/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   System.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/24 17:50:21 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/24 18:12:10 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

	#include "Config/Config.hpp"
	#include "Utils/Utils.hpp"

	#include <sys/resource.h>													// getrlimit(), setrlimit()

#pragma region "Check FD Limit"

	int check_fd_limit(uint16_t minfds) {
		struct rlimit rl;

		if (!getrlimit(RLIMIT_NOFILE, &rl)) {
			if (rl.rlim_cur > minfds) return (0);
			else if (Config::is_root && rl.rlim_cur < rl.rlim_max && minfds <= rl.rlim_max) {
				rl.rlim_cur = minfds;
				return (setrlimit(RLIMIT_NOFILE, &rl) != 0);
			}
		}

		return (1);
	}

#pragma endregion

#pragma region "Check Process Limit"

	int check_process_limit(uint16_t minprocs) {
		struct rlimit rl;

		if (getrlimit(RLIMIT_NPROC, &rl) == 0) {
			if (rl.rlim_cur > minprocs) return (0);
			else if (Config::is_root && rl.rlim_cur < rl.rlim_max && minprocs <= rl.rlim_max) {
				rl.rlim_cur = minprocs;
				return (setrlimit(RLIMIT_NOFILE, &rl) != 0);
			}
		}

		return (1);
	}

#pragma endregion