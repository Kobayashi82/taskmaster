/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/11 19:29:12 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/10 14:29:10 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Main/Taskmasterd.hpp"
	#include "Main/Signal.hpp"
	#include "Loop/Epoll.hpp"

	#include <iostream>															// std::cerr()

#pragma endregion

#pragma region "Main"

	int main(int argc, char **argv) {
		int result = 0;

		Signal::set_for_load();

		if ((result = Config.load(argc, argv))) {
			Log.set_logfile(TaskMaster.logfile);
			Log.set_logfile_ready(true);
			TaskMaster.clean_up();
			return (result - 1);
		}

		Pidfile pidfile(TaskMaster.pidfile);
		TaskMaster.pidfile_ptr = &pidfile;
		if (pidfile.is_locked()) {
			std::cerr << "Error: another taskmasterd instance is already running" << std::endl;
			return (1);
		}

		std::remove(TaskMaster.logfile.c_str());	// REMOVE

		Log.set_logfile(TaskMaster.logfile);
		Log.set_logfile_ready(true);

		result = daemonize(pidfile);
		if (result == -1)	{ TaskMaster.clean_up();	return (0);					   }
		if (result)			{ TaskMaster.clean_up();	return (result);			   }
		if (Signal::signum)	{ TaskMaster.clean_up();	return (128 + Signal::signum); }

		Log.info("Taskmasterd: started with pid " + std::to_string(TaskMaster.pid));

		int sigfd = Signal::create();
		if (sigfd == -1) {
			TaskMaster.clean_up();
			return (1);
		}

		Epoll epoll;
		TaskMaster.epoll_ptr = &epoll;
		if (epoll.create() || epoll.add(sigfd, true, false)) {
			TaskMaster.clean_up();
			return (1);
		}

		TaskMaster.unix_server.start();
		TaskMaster.inet_server.start();

		while (TaskMaster.running) {
			// Máquina de estados
			if (epoll.wait()) break;
		}

		TaskMaster.clean_up();

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
