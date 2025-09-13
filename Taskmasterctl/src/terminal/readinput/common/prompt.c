/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   prompt.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/05 09:44:04 by vzurera-          #+#    #+#             */
/*   Updated: 2025/02/27 12:29:24 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "libft.h"
	#include "terminal/readinput/termcaps.h"
	#include "terminal/readinput/prompt.h"
	#include "terminal/colors.h"
	#include "main/options.h"

#pragma endregion

#pragma region "Variables"

	#define PS1_DFLT		BLUE600"["GREEN600"kobayashi"BLUE600"]"GREEN600"-"RED600"42"Y"sh"BLUE600"> "NC
	#define PS2_DFLT		"> "

	char	*prompt_PS1;			//	Default prompt displayed for regular input
	char	*prompt_PS2;			//	Continuation prompt for heredocs or multiline input

#pragma endregion

#pragma region "Utils"

	#pragma region "Backslashes"

		static char *backslashes(char *str) {
			char *tmp = str;

			while (tmp && *tmp) {
				if (*tmp == '\\' && *(tmp + 1) == '\\')
					ft_memmove(tmp, tmp + 1, ft_strlen(tmp + 1) + 1);
				tmp++;
			}

			return (str);
		}

	#pragma endregion

#pragma endregion

#pragma region "Set"

	#pragma region "Remove Forbidden Chars"

		void remove_forbidden_chars(char *new_prompt) {
			if (!new_prompt) return;

			char *tmp = smalloc(ft_strlen(new_prompt) + 1);
			size_t i = 0, j = 0;

			while (new_prompt[i]) {
				int len = char_size(new_prompt[i]);

				if (i + len > ft_strlen(new_prompt)) break;
				if (len == 0) { i++; continue; }

				if (char_width(i, new_prompt) < 2) {
					ft_memcpy(&tmp[j], &new_prompt[i], len);
					j += len;
				} i += len;
			} tmp[j] = '\0';

			ft_strlcpy(new_prompt, tmp, ft_strlen(tmp) + 1);
			sfree(tmp);
		}

	#pragma endregion

	#pragma region "Set"

		void prompt_set(int type, char *new_prompt) {
			// char *str_PS1 = "\\\\\\$USER-\\u > ";						//	"\\\$USER-\u"
			char *tmp_prompt = NULL;

			prompt_clear(type);

			if (new_prompt) {
				//	Procesa barras, variables ($var & \u)
				tmp_prompt = ft_strdup(new_prompt);
			} else {
				if (type == PS1) tmp_prompt = backslashes(ft_strdup(PS1_DFLT));	//	"\\$USER-\u"
				if (type == PS2) tmp_prompt = backslashes(ft_strdup(PS2_DFLT));	//	"\\$USER-\u"
				//	Procesa barras, variables ($var & \u)
			}

			if (!options.multiwidth_chars) remove_forbidden_chars(tmp_prompt);

			if (type == PS1) prompt_PS1 = tmp_prompt;
			if (type == PS2) prompt_PS2 = tmp_prompt;
		}

	#pragma endregion

#pragma endregion

#pragma region "Clear"

	void prompt_clear(int type) {
		if ((type == PS1 || type == BOTH) && prompt_PS1) { sfree(prompt_PS1); prompt_PS1 = NULL; }
		if ((type == PS2 || type == BOTH) && prompt_PS2) { sfree(prompt_PS2); prompt_PS2 = NULL; }
	}

#pragma endregion

#pragma region "Initialize"

	int prompt_initialize() {
		prompt_set(PS1, NULL);
		prompt_set(PS2, NULL);

		return (0);
	}

#pragma endregion

#pragma region "Information"

	//	PROMPT_COMMAND			Ejecuta antes de mostrar el prompt

	// \d	Fecha en formato día mes fecha (e.g., Tue Mar 7).
	// \h	Nombre del host hasta el primer punto (e.g., mi-host).
	// \H	Nombre completo del host (e.g., mi-host.localdomain).
	// \j	Número de trabajos en segundo plano manejados por el shell.
	// \l	Nombre del terminal en el que estás (e.g., tty1, pts/0).
	// \s	Nombre del shell (e.g., bash).
	// \t	Hora actual en formato 24 horas (hh:mm:ss).
	// \T	Hora actual en formato 12 horas (hh:mm:ss).
	// \@	Hora actual en formato 12 horas con AM/PM (hh:mm AM).
	// \u	Nombre del usuario actual.
	// \v	Versión de 42sh
	// \V	Versión de 42sh
	// \w	Directorio de trabajo actual (con rutas cortas, e.g., ~/Documentos).
	// \W	Nombre base del directorio actual (solo el último componente, e.g., Documentos).
	// \!	Número del comando actual en el historial.
	// \#	Número del comando actual en esta sesión.
	// \$	Muestra # si eres root, o $ si eres un usuario normal.

	// \a	Carácter de campana (alerta sonora).
	// \n	Nueva línea.
	// \r	Retorno de carro.
	// \e	Carácter de escape ASCII (código 033, útil para colores).
	// \[	Inicia una secuencia de caracteres no imprimibles (útil para colores y estilos).
	// \]	Finaliza una secuencia de caracteres no imprimibles.

#pragma endregion
