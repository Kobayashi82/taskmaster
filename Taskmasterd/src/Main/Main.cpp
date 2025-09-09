/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/11 19:29:12 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/09 20:18:46 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Main/Taskmasterd.hpp"
	#include "Main/Signals.hpp"

	#include <unistd.h>															// execvpe()
	#include <signal.h>															// signal()
	#include <iostream>															// std::cerr()

#pragma endregion

#pragma region "Main"

	int main(int argc, char **argv) {
		int result = 0;

		Signals::set_for_load();

		if ((result = Config.load(argc, argv))) {
			Log.set_logfile(TaskMaster.logfile);
			Log.set_logfile_ready(true);
			return (result - 1);
		}

		Pidfile pidfile(TaskMaster.pidfile);
		if (pidfile.is_locked()) {
			std::cerr << "Error: another taskmasterd instance is already running\n";
			return (1);
		}

		sleep(5);
		std::remove(TaskMaster.logfile.c_str());		// REMOVE
		Log.set_logfile(TaskMaster.logfile);
		Log.set_logfile_ready(true);

		// Daemonize
		result = daemonize(pidfile);
		if (result == -1)	return (0);
		if (result)			{
			pidfile.unlock();
			Log.info("Taskmasterd: closed");
			Log.close();
			return (result);
		}

		if (Signals::signum) {
			pidfile.unlock();
			Log.info("Taskmasterd: closed");
			Log.close();
			return (128 + Signals::signum);
		}

		Log.info("Taskmasterd: started with pid " + std::to_string(getpid()));

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

		pidfile.unlock();
		Log.info("Taskmasterd: closed");
		Log.close();

		if (!result && Signals::signum) result = 128 + Signals::signum;
		return (result);
	}

#pragma endregion
