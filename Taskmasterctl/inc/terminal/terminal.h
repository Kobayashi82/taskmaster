/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   terminal.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/29 13:40:29 by vzurera-          #+#    #+#             */
/*   Updated: 2025/03/04 20:37:22 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region Includes

	#include <stddef.h>
	#include <termios.h>

#pragma endregion

#pragma region Variables

	#pragma region Structures

		typedef struct termios	t_termios;
		typedef struct {
			t_termios	term;
			size_t		rows;
			size_t		cols;
			char		*input;

			char 		*PS1;
			char 		*PS2;
			char		*prompt;

			int			bk_stdin;
			int			bk_stdout;
			int			bk_stderr;

			char		quoted;
		} t_terminal;

	#pragma endregion

	extern t_terminal	terminal;

#pragma endregion
