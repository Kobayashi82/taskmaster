/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/11 19:29:12 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/05 10:37:15 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Utils/Utils.hpp"
	#include "Config/Options.hpp"
	#include "Config/Config.hpp"
	#include "Programs/TaskManager.hpp"
	#include "Logging/TaskmasterLog.hpp"

	#include <iostream>															// std::cerr()
	#include <unistd.h>

#pragma endregion

#pragma region "Main"

	int main(int argc, char **argv) {
		int result = 0;

		if ((result = Config.load(argc, argv))) return (result) - 1;

		for (auto& program : TaskMaster.programs) {
			for (auto& process : program.process) {
				std::cerr << process.name << "\n";
			}
		}

		char **envp = Utils::toArray(TaskMaster.programs[0].process[0].environment);
		char **args = Utils::toArray({"-a", "-l"});

		Log.close();
		execvpe(TaskMaster.programs[2].process[0].command.c_str(), args, envp);
		Utils::array_free(envp);
		Utils::array_free(args);

		// std::cout << TaskMaster.programs[1].process[0].command << "\n";
		// std::cout << TaskMaster.programs[0].groups[0] << "\n";
		// std::cout << TaskMaster.directory << "\n";

		Log.info("cerrando");

		return (result);
	}

#pragma endregion
