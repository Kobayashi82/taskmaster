/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   readinput.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/05 09:44:40 by vzurera-          #+#    #+#             */
/*   Updated: 2025/03/02 19:36:20 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

//	Auto-Complete
//	Sintaxis
//	Prompt
//	Command with char position (los comandos de vi que dependen de una posicion... duh)

//	Current issues:
//
//	Incomplete support for multi-width characters (missing logic to handle them at column edges).		SOLUTION: Multi-width characters are ignored.
//	When the input has more lines than the terminal, the cursor movement logic breaks.					SOLUTION: Do not write long lines.
//	Historial de varias lineas no se pega a la columna 0.

#pragma region "Includes"

	#include "libft.h"
	#include "terminal/terminal.h"
	#include "terminal/readinput/termcaps.h"
	#include "terminal/readinput/prompt.h"
	#include "terminal/readinput/readinput.h"
	#include "terminal/readinput/history.h"
	#include "main/options.h"
	#include "main/error.h"

#pragma endregion

#pragma region "Variables"

	t_buffer	buffer;

	char		*term_prompt;
	bool		raw_mode;
	int			vi_mode;

#pragma endregion

#pragma region "Raw Mode"

	void disable_raw_mode() {
		if (raw_mode) {
			raw_mode = false;
			cursor_show();
			terminal_release();
			if (fcntl(STDIN_FILENO, F_GETFD) == -1) {
				int tty_fd = open("/dev/tty", O_RDWR);
				if (tty_fd == -1) {
					sfree(buffer.value);
					write(STDERR_FILENO, "\n", 1);
					exit_error(STDIN_CLOSED, 1, NULL, true);
				}
				tcsetattr(tty_fd, TCSAFLUSH, &terminal.term);
				sdup2(&tty_fd, STDIN_FILENO, true);
				write(STDOUT_FILENO, "\n", 1);
			} else
				tcsetattr(STDIN_FILENO, TCSAFLUSH, &terminal.term);
			sfree(term_prompt); term_prompt = NULL;
		}
	}

	static void enable_raw_mode() {
		raw_mode = true;
		tcgetattr(STDIN_FILENO, &terminal.term);
		terminal_initialize();

		struct termios raw = terminal.term;
		raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);			//	Disable echo (ECHO), canonical mode (ICANON), signals (ISIG), and extended input processing (IEXTEN)
		raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);	//	Disable break interrupt (BRKINT), carriage return to newline conversion (ICRNL), parity check (INPCK), stripping of eighth bit (ISTRIP), and software flow control (IXON)
		raw.c_oflag &= ~(OPOST);									//	Disable post-processing of output (OPOST)
		raw.c_cc[VMIN] = 1;											//	Read at least 1 character before returning
		raw.c_cc[VTIME] = 0;										//	No timeout for read

		tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
	}

#pragma endregion

#pragma region "Check TTY"

	static int check_tty() {
		if (fcntl(STDIN_FILENO, F_GETFD) == -1) {
			int tty_fd = open("/dev/tty", O_RDWR);
			if (tty_fd == -1) {
				sfree(buffer.value);
				disable_raw_mode();
				write(STDERR_FILENO, "\n", 1);
				exit_error(STDIN_CLOSED, 1, NULL, true);
			}
			sdup2(&tty_fd, STDIN_FILENO, true);
			return (1);
		}
		
		if (fcntl(STDOUT_FILENO, F_GETFD) == -1) {
			int tty_fd = open("/dev/tty", O_WRONLY);
			if (tty_fd != -1) {
				sfree(buffer.value);
				disable_raw_mode();
				write(STDERR_FILENO, "\n", 1);
				exit_error(STDOUT_CLOSED, 1, NULL, true);
			}
			sdup2(&tty_fd, STDOUT_FILENO, true);
		}

		return (0);
	}

#pragma endregion

#pragma region "ReadInput"

	char *readinput(char *prompt) {
		int result = 0;
		buffer.size = 1024;
		buffer.position = 0, buffer.length = 0;
		buffer.value = ft_calloc(buffer.size, sizeof(char));
		buffer.CTRL = false; buffer.ALT = false; buffer.SHIFT = false;

		enable_raw_mode();

		sfree(term_prompt);
		term_prompt = ft_strdup(prompt);

		if (term_prompt) write(STDOUT_FILENO, term_prompt, ft_strlen(term_prompt));
		vi_mode = 0;

		cursor_get();
		while (!result) {
			cursor_show();
			int readed = read(STDIN_FILENO, &buffer.c, 1);
			if (check_tty()) continue;
			cursor_hide();		

			if (hist_searching && history_search()) continue;

			if		(options.emacs)	result = readline(readed);
			else if	(options.vi)	result = vi();
			else					result = dumb(readed);
		}
		
		history_set_pos_end();
		undo_clear();
		
		disable_raw_mode();
		
		return (buffer.value);
	}

#pragma endregion
