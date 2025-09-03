/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Group.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/02 17:23:05 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/03 19:40:41 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Config.hpp"
	#include "Programs/Manager.hpp"
	#include "Programs/Group.hpp"

	#include <sstream>															// std::stringstream

#pragma endregion

#pragma region "Constructors"

	Group::Group(const std::string _section) : section(_section), name(_section.substr(6)) {
		
		std::set<std::string> program_names;
		std::stringstream ss(Config.get_value(section, "programs")); std::string token;
		while (std::getline(ss, token, ',')) program_names.insert(token);

		for (auto& program : Manager.Programs) {
			if (program_names.find(program.name) != program_names.end()) {
				programs.push_back(&program);
				program.groups.push_back(this);
			}
		}
	}

#pragma endregion
