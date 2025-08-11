/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/11 19:29:12 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/11 20:01:53 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "main/taskmaster.h"

	#include <signal.h>

#pragma endregion

#pragma region "Variables"

	t_taskmaster g_taskmaster;

#pragma endregion

#pragma region "Signals"

	static void termination_handler(int sig) { (void) sig; g_taskmaster.running = false; }

	static void set_signals() {
		signal(SIGINT,  termination_handler);
		signal(SIGQUIT, termination_handler);
		signal(SIGTERM, termination_handler);
		signal(SIGHUP,  termination_handler);
	}

#pragma endregion

#pragma region "Initialize"

	void initialize(char *arg) {
		g_taskmaster.fullname = arg;
	}

#pragma endregion

#pragma region "Main"

	int main(int argc, char **argv) {
		int result = 0;

		initialize(argv[0]);
		if ((result = parse_options(&g_taskmaster.options, argc, argv))) return (result == 2);
		set_signals();

		return (result);
	}

#pragma endregion
