/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/11 19:29:12 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/07 13:46:46 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Utils/Utils.hpp"
	#include "Config/Config.hpp"
	#include "Programs/TaskManager.hpp"
	#include "Logging/TaskmasterLog.hpp"

	#include <iostream>															// std::cerr()
	#include <unistd.h>															// execvpe()
	#include <signal.h>

#pragma endregion

void reload_signal(int signum) {
	(void) signum;
	Config.reload();
	TaskMaster.silent = true;
}

#pragma region "Main"

	int main(int argc, char **argv) {
		int result = 0;

		if ((result = Config.load(argc, argv))) return (result) - 1;

		std::cerr << getpid() << "\n";
		std::cerr << TaskMaster.programs[2].process[0].name << "\n";
		
		signal(SIGHUP, reload_signal);

		while (!TaskMaster.silent) ;

		std::cerr << TaskMaster.programs[2].process[0].name << "\n";

		Log.close();
		char **envp = Utils::toArray(TaskMaster.programs[2].process[0].environment);
		char **args = Utils::toArray(TaskMaster.programs[2].process[0].arguments);

		// Utils::environment_print(TaskMaster.programs[2].process[0].environment);

		TaskMaster.unix_server.close();
		TaskMaster.inet_server.close();
		unlink(TaskMaster.unix_server.file.c_str());
		unlink(TaskMaster.pidfile.c_str());
		execvpe(TaskMaster.programs[2].process[0].command.c_str(), args, envp);
		Utils::array_free(envp);
		Utils::array_free(args);
		
		Log.info("cerrando");

		return (result);
	}

#pragma endregion
