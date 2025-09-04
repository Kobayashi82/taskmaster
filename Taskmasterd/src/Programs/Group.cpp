/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Group.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/02 17:23:05 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/04 23:45:42 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Config.hpp"
	#include "Programs/TaskManager.hpp"
	#include "Programs/Group.hpp"

	#include <sstream>															// std::stringstream

#pragma endregion

#pragma region "Constructors"

	Group::Group(const std::string _section) : section(_section), name(_section.substr(6)) {

		std::set<std::string> program_names;
		std::stringstream ss(Config.get_value(section, "programs")); std::string token;
		while (std::getline(ss, token, ',')) program_names.insert(token);

		for (auto& program : TaskMaster.programs) {
			if (program_names.find(program.name) != program_names.end()) {
				if (!program.disabled) {
					// Añadir a program.process la variable del grupo
					programs.push_back(program.name);
					program.groups.push_back(name);
				}
			}
		}
	}

#pragma endregion
