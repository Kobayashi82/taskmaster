/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   readinput.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/29 13:40:29 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/13 23:16:20 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include <stddef.h>
	#include <stdbool.h>

#pragma endregion

#pragma region "Variables"

	#pragma region "Enumerators"

		enum e_input_mode { READLINE, VI };

	#pragma endregion

	#pragma region "Structures"

		typedef struct s_buffer {
			unsigned char	c;
			char			*value;
			size_t			size, position, length;
			bool			SHIFT, ALT, CTRL;
		}	t_buffer;

	#pragma endregion

	extern t_buffer	buffer;
	extern char		*term_prompt;
	extern bool		raw_mode;
	extern bool		hist_searching;
	extern int		vi_mode;

#pragma endregion

#pragma region "Methods"

	//	--------- READINPUT --------
	void	disable_raw_mode();
	char	*readinput(char *prompt);
	char	*get_input();

	//	---------- MODES -----------
	int		readline(int readed);

	//	------- AUTOCOMPLETE -------
	void	autocomplete();

	//	------ HISTORY SEARCH ------
	int		history_search();

	//	-------- UNDO/REDO ---------
	void	undo_push(bool push);
	void	undo_pop();
	void	undo_all();
	void	undo_clear();

#pragma endregion
