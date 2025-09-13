/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signal.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/29 15:57:35 by vzurera-          #+#    #+#             */
/*   Updated: 2025/02/23 17:08:42 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "libft.h"
	#include "terminal/terminal.h"
	#include "terminal/readinput/termcaps.h"
	#include "terminal/readinput/readinput.h"
	#include "terminal/signals.h"
	#include "main/shell.h"
	#include "main/options.h"
	#include "main/error.h"

	#include <signal.h>
	#include <sys/ioctl.h>

#pragma endregion

int	nsignal;

#pragma region "SIG_INT"

	//	Handle SIGINT signal
	static void sigint_handler(int sig) {
		shell.exit_code = 128 + sig;
		nsignal = sig;
		if (raw_mode) {
			char byte = 3;
			ioctl(STDIN_FILENO, TIOCSTI, &byte);
		}
	}

#pragma endregion

#pragma region "SIG_QUIT"

	//	Handle SIGQUIT signal
	// static void sigquit_handler(int sig) {
	// 	nsignal = sig;
	// 	// if (raw_mode) {
	// 	// 	disable_raw_mode();
	// 	// 	write(1, "\n", 1);
	// 	// 	if (buffer.value) sfree(buffer.value);
	// 	// }
	// 	// exit_error(SEGQUIT, 3, NULL, true);
	// }

#pragma endregion

#pragma region "SIG_WINCH"

	//	Handle SIGWINCH signal
	static void sigwinch_handler(int sig) {
		(void) sig;
		terminal_initialize();
		if (raw_mode) cursor_get();
	}

#pragma endregion

#pragma region "SIG_SEV"

	//	Handle SIGSEGV signal
	static void sigsegv_handler(int sig) {
		shell.exit_code = 128 + sig;
		nsignal = sig;
		if (raw_mode) {
			disable_raw_mode();
			write(1, "\n", 1);
			if (buffer.value) sfree(buffer.value);
		}
		exit_error(SEGFAULT, 11, NULL, true);
	}

#pragma endregion

#pragma region "Set"

	void signals_set() {
		signal(SIGINT, sigint_handler);
		//signal(SIGQUIT, sigquit_handler);
		signal(SIGQUIT, SIG_IGN);
		signal(SIGWINCH, sigwinch_handler);
		signal(SIGSEGV, sigsegv_handler);
		nsignal = 0;
	}

#pragma endregion

// kill -SIGSEGV $(pgrep 42sh)
// kill -SIGSEGV $(pgrep -f 'valgrind.*42sh')