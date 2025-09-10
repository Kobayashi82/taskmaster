/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Event.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/18 11:21:01 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/10 21:32:14 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Taskmaster/Taskmaster.hpp"

#pragma endregion

#pragma region "Get"

	EventInfo* Event::get(int fd) {
		if (fd < 0) return (nullptr);

		auto it = events.find(fd);
		if (it != events.end())	return (&it->second);

		return (nullptr);
	}

#pragma endregion

#pragma region "Clear"

	void Event::clear() {
		events.clear();
	}

#pragma endregion

#pragma region "Remove (FD)"

	void Event::remove(int fd) {
		events.erase(fd);
	}

#pragma endregion

#pragma region "Remove (Owner)"

	void Event::remove(void *owner) {
		for (auto it = events.begin(); it != events.end();) {
			if (it->second.owner.has_value()) {
				try {
					if (std::any_cast<void *>(it->second.owner) == static_cast<void *>(owner)) {
						it = events.erase(it);
						continue;
					}
				} catch (const std::bad_any_cast&) {}
			}
			++it;
		}
	}

#pragma endregion
