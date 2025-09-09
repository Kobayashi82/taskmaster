/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/11 19:29:12 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/09 14:58:49 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Main/Taskmasterd.hpp"

	#include <unistd.h>															// execvpe()
	#include <signal.h>															// signal()
	#include <iostream>															// std::cerr()

#pragma endregion

void reload_signal(int signum) {
	(void) signum;
	Config.reload();
	TaskMaster.silent = true;
}

#pragma region "Main"

	int main(int argc, char **argv) {
		int result = 0;

		signal(SIGHUP, reload_signal);

		if ((result = Config.load(argc, argv))) return (result) - 1;

		Pidfile pidfile(TaskMaster.pidfile);
		if (pidfile.is_locked()) {
			Log.critical("Pidfile: lock already in use");
			return (1);
		}

		// Daemonize
		result = daemonize(pidfile);
		if (result == -1)	return (0);
		if (result)			return (result);

		TaskMaster.unix_server.start();
		TaskMaster.inet_server.start();

		// std::cerr << getpid() << "\n";
		// std::cerr << TaskMaster.programs[2].process[0].name << "\n";
		// while (!TaskMaster.silent) ;
		// std::cerr << TaskMaster.programs[2].process[0].name << "\n";

		// Log.close();
		// char **envp = Utils::toArray(TaskMaster.programs[2].process[0].environment);
		// char **args = Utils::toArray(TaskMaster.programs[2].process[0].arguments);

		// Utils::environment_print(TaskMaster.programs[2].process[0].environment);

		TaskMaster.unix_server.close();
		TaskMaster.inet_server.close();
		std::remove(TaskMaster.unix_server.file.c_str());
		std::remove(TaskMaster.pidfile.c_str());

		// execvpe(TaskMaster.programs[2].process[0].command.c_str(), args, envp);

		// Utils::array_free(envp);
		// Utils::array_free(args);

		Log.info("Taskmasterd: closing");
		Log.close();
		pidfile.unlock();

		// if (!result && TaskMaster.signum) result = 128 + TaskMaster.signum;
		return (result);
	}

#pragma endregion
