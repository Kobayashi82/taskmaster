/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   taskmaster.h                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/11 19:11:10 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/11 20:01:28 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include "main/options.h"

#pragma endregion

#pragma region "Structures"

	typedef struct s_taskmaster {
		bool			running;
		char			*fullname;
		t_options		options;
	}	t_taskmaster;

#pragma endregion

#pragma region "Variables"

	extern t_taskmaster g_taskmaster;

#pragma endregion

#pragma region "Methods"



#pragma endregion
