/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/11 19:29:12 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/10 22:11:46 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Taskmaster/Taskmaster.hpp"

	#include <iostream>															// std::cerr()

#pragma endregion

#pragma region "Main"

	int main(int argc, char **argv) {
		int result = 0;

		Signal::set_for_load();

		if ((result = Config.load(argc, argv))) {
			Log.set_logfile(tskm.logfile);
			Log.set_logfile_ready(true);
			tskm.cleanup();
			return (result - 1);
		}

		if (tskm.pidlock.is_locked(tskm.pidfile)) {
			std::cerr << "Error: another taskmasterd instance is already running" << std::endl;
			return (1);
		}

		// std::remove(tskm.logfile.c_str());	// REMOVE

		Log.set_logfile(tskm.logfile);
		Log.set_logfile_ready(true);

		result = tskm.daemonize();
		if (result == -1)	{ tskm.cleanup(true);	return (0);					   }
		if (result)			{ tskm.cleanup();		return (result);			   }
		if (Signal::signum)	{ tskm.cleanup();		return (128 + Signal::signum); }

		Log.info("Taskmasterd: started with pid " + std::to_string(tskm.pid));

		int sigfd = Signal::create();
		if (sigfd == -1) {
			tskm.cleanup();
			return (1);
		}

		if (tskm.epoll.create() || tskm.epoll.add(sigfd, true, false)) {
			tskm.cleanup();
			return (1);
		}

		tskm.unix_server.start();
		tskm.inet_server.start();

		while (tskm.running) {
			// Máquina de estados
			if (tskm.epoll.wait()) break;
		}

		std::cout << "WTF " << std::to_string(tskm.inet_server.sockfd) << std::endl;
		tskm.cleanup();

		return ((Signal::signum) ? 128 + Signal::signum : result);
	}

#pragma endregion

	// std::cerr << getpid() << "\n";
	// std::cerr << TaskMaster.programs[2].process[0].name << "\n";
	// while (!TaskMaster.silent) ;
	// std::cerr << TaskMaster.programs[2].process[0].name << "\n";

	// Log.close();
	// char **envp = Utils::toArray(TaskMaster.programs[2].process[0].environment);
	// char **args = Utils::toArray(TaskMaster.programs[2].process[0].arguments);

	// Utils::environment_print(TaskMaster.programs[2].process[0].environment);
	
	// execvpe(TaskMaster.programs[2].process[0].command.c_str(), args, envp);

	// Utils::array_free(envp);
	// Utils::array_free(args);
