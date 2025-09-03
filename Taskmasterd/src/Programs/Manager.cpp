/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Manager.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/02 17:23:05 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/03 19:42:29 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Config.hpp"
	#include "Programs/Manager.hpp"

#pragma endregion

#pragma region "Variables"

	ProgramManager Manager;

#pragma endregion

void ProgramManager::initialize() {
	for (auto& [program, keys] : Config.sections) {
		if (program.substr(0, 8) == "program:") {
			if (Config.get_value(program, "command").empty()) continue;
			Programs.emplace_back(program);
		}
	}

	for (auto& [group, keys] : Config.sections) {
		if (group.substr(0, 6) == "group:") {
			if (Config.get_value(group, "programs").empty()) continue;
			Groups.emplace_back(group);
		}
	}
}
