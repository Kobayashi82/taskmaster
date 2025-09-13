/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dumb.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/11 16:34:33 by vzurera-          #+#    #+#             */
/*   Updated: 2025/02/25 16:59:10 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "libft.h"
	#include "terminal/terminal.h"
	#include "terminal/readinput/readinput.h"
	#include "terminal/readinput/prompt.h"
	#include "terminal/readinput/history.h"
	#include "terminal/signals.h"
	#include "main/shell.h"
	#include "main/options.h"

#pragma endregion

#pragma region "Input"

	#pragma region "Insert"

		#pragma region "EOF"							("CTRL + D")

			static int ctrl_d(const int n) {
				if (n <= 0 || (buffer.c == 4 && !buffer.length)) {
					sfree(buffer.value); buffer.value = NULL;
					write(STDOUT_FILENO, "\r\n", 2);

					return (1);
				} return (0);
			}

		#pragma endregion

		#pragma region "SIG_INT"						("CTRL + C")

			static int ctrl_c() {
				if (buffer.c == 3) {
					buffer.value[0] = '\0'; buffer.position = 0; buffer.length = 0;

					if (options.hide_ctrl_chars)	write(STDOUT_FILENO, "\r\n", 2);
					else							write(STDOUT_FILENO, "^C\r\n", 4);

					shell.exit_code = 130; nsignal = 2;
					return (1);
				} return (0);
			}

		#pragma endregion

		#pragma region "NewLine"						("Enter")

			static int enter() {
				if (buffer.c == '\r' || buffer.c == '\n') {
					buffer.value[buffer.length] = '\0';

					write(STDOUT_FILENO, "\r\n", 2);

					return (1);
				} return (0);
			}

		#pragma endregion

		#pragma region "Char"

			static int print_char() {
				size_t c_size = 1;
				if 		(buffer.c >= 0xF0)	c_size = 4;
				else if (buffer.c >= 0xE0)	c_size = 3;
				else if (buffer.c >= 0xC0)	c_size = 2;

				// Expand buffer if necessary
				if (buffer.position + c_size >= buffer.size - 1) {
					buffer.value = ft_realloc(buffer.value, buffer.size, buffer.size * 2);
					buffer.size *= 2;
				}

				// Insert all bytes of the character into the buffer
				buffer.value[buffer.position++] = buffer.c;
				for (size_t i = 1; i < c_size; i++) read(STDIN_FILENO, &buffer.value[buffer.position++], 1);
				buffer.length += c_size;
				buffer.value[buffer.length] = '\0';

				write(STDOUT_FILENO, &buffer.value[buffer.position - c_size], buffer.length - (buffer.position - c_size));

				return (0);
			}

		#pragma endregion

	#pragma endregion

	#pragma region "Handle"

		#pragma region "Navigation"

			#pragma region "Cursor"

				static int cursor() {
					char seq[8];
					if (buffer.c == 27) {

						fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
						read(STDIN_FILENO, seq, sizeof(seq) - 1);
						fcntl(STDIN_FILENO, F_SETFL, O_SYNC);

						return (1);
					} return (0);
				}

			#pragma endregion
	
		#pragma endregion

		#pragma region "Specials"

			static int specials() {
				if		(buffer.c >= 1 && buffer.c <= 26)	{ ;								}	//	Ignore all CTRL + X commands
				else if (buffer.c >= 28 && buffer.c <= 31)	{ ;								}	//	Ignore other CTRL + X commands
				else return (0);

				return (1);
			}

		#pragma endregion

	#pragma endregion

#pragma endregion

#pragma region "Dumb"

	int dumb(int readed) {
		int result = 0;

		if		(ctrl_d(readed))	result = 1;
		else if	(ctrl_c())			result = 0;
		else if	(enter())			result = 1;
		else if (specials())		result = 0;
		else if (cursor())			result = 0;
		else if (print_char())		result = 0;

		return (result);
	}

#pragma endregion
