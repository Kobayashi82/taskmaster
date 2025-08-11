/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   options.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/11 19:11:17 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/11 20:01:35 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#define _GNU_SOURCE

	#include <errno.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <unistd.h>
	#include <limits.h>
	#include <getopt.h>
	#include <stdbool.h>
	#include <sys/time.h>

#pragma endregion

#pragma region "Defines"

	#define NAME	"taskmaster"

#pragma endregion

#pragma region "Structures"

	typedef struct s_options {
		int					pid;
		unsigned long		options;
		size_t				example;
	}	t_options;

#pragma endregion

#pragma region "Methods"

	int	parse_options(t_options *options, int argc, char **argv);

#pragma endregion
