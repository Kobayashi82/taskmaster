/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   System.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/24 17:50:21 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/25 14:42:09 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

	#include "Utils/Utils.hpp"

	#include <unistd.h>															// getuid()
	#include <sys/resource.h>													// getrlimit(), setrlimit()

#pragma region "Check FD Limit"

	int check_fd_limit(uint16_t minfds) {
		struct rlimit rl;

		if (!getrlimit(RLIMIT_NOFILE, &rl)) {
			if (rl.rlim_cur > minfds) return (0);
			else if (getuid() == 0 && rl.rlim_cur < rl.rlim_max && minfds <= rl.rlim_max) {
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
			else if (getuid() == 0 && rl.rlim_cur < rl.rlim_max && minprocs <= rl.rlim_max) {
				rl.rlim_cur = minprocs;
				return (setrlimit(RLIMIT_NOFILE, &rl) != 0);
			}
		}

		return (1);
	}

#pragma endregion