/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   prompt.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/10 20:53:31 by vzurera-          #+#    #+#             */
/*   Updated: 2025/02/25 13:59:24 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Variables"

	#pragma region "Enumerators"

		enum e_prompt_type { PS1, PS2, BOTH };

	#pragma endregion

	extern char		*prompt_PS1;
	extern char		*prompt_PS2;

#pragma endregion

#pragma region "Methods"

	//	---------- PROMPT ----------
	void	prompt_set(int type, char *new_prompt);
	void	prompt_clear(int type);
	int		prompt_initialize();

#pragma endregion
