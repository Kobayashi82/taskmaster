/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   input.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/25 15:02:36 by vzurera-          #+#    #+#             */
/*   Updated: 2025/03/14 09:29:08 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

//	alias echo=date c=lala d=lolo b='lele ' f=lili g=h h='j b d ' j='h'
//	c ; d ; (g) d d | f $(( (b) $(echo `b g` $((b * 3)) ) + $(b b 3 | wc -l) )) || d

#pragma region "Includes"

	#include "libft.h"
	#include "terminal/readinput/prompt.h"
	#include "terminal/readinput/readinput.h"
	#include "terminal/readinput/history.h"
	#include "terminal/input.h"
	#include "terminal/signals.h"
	#include "parser/expansions/alias.h"
	#include "parser/expansions/history.h"
	#include "parser/syntax/syntax.h"
	#include "main/shell.h"

#pragma endregion

#pragma region "Expand"

	char *expand_input(char *input) {
		if (!input || ft_isspace_s(input)) return (input);

		char *input_hist = NULL;
		int line = 1;

		t_context ctx_history;		ft_memset(&ctx_history, 0, sizeof(t_context));
		t_context ctx_syntax;		ft_memset(&ctx_syntax, 0, sizeof(t_context));
		t_context ctx_alias;		ft_memset(&ctx_alias, 0, sizeof(t_context));

		expand_history(&input, &ctx_history, true);
		input_hist = ft_strdup(input);

		expand_alias(&input, &ctx_alias);
		context_copy(&ctx_history, &ctx_alias);

		if (syntax_check(input, &ctx_syntax, line)) {
			sfree(input); sfree(input_hist);
			stack_clear(&ctx_history.stack);
			stack_clear(&ctx_syntax.stack);
			stack_clear(&ctx_alias.stack);
			return (ft_strdup(""));
		}
		
		while (shell.interactive && (ctx_history.in_token || ctx_history.stack)) {
			bool add_newline = !ctx_history.in_token;
			bool is_escape = ctx_history.in_escape;
			if (add_newline) line++;

			char *cont_line = readinput(prompt_PS2);
			if (!cont_line) {
				sfree(input); sfree(input_hist);
				stack_clear(&ctx_history.stack);
				stack_clear(&ctx_syntax.stack);
				stack_clear(&ctx_alias.stack);
				return (NULL);
			}

			if (!*cont_line && nsignal == 2) {
				sfree(input); sfree(input_hist);
				stack_clear(&ctx_history.stack);
				stack_clear(&ctx_syntax.stack);
				stack_clear(&ctx_alias.stack);
				return (cont_line);
			}

			expand_history(&cont_line, &ctx_history, true);

			if (add_newline)
				input_hist = ft_strjoin_sep(input_hist, "\n", cont_line, 1);
			else if (is_escape) {
				size_t len = ft_strlen(input_hist);
				while (len && ft_isspace(input_hist[len - 1])) input_hist[--len] = '\0';
				if (input_hist[len - 1] == '\\') input_hist[--len] = '\0';
				input_hist = ft_strjoin(input_hist, cont_line, 1);
			} else
				input_hist = ft_strjoin_sep(input_hist, " ", cont_line, 1);

			expand_alias(&cont_line, &ctx_alias);
			context_copy(&ctx_history, &ctx_alias);

			if (syntax_check(cont_line, &ctx_syntax, line)) {
				sfree(input); sfree(cont_line); sfree(input_hist);
				stack_clear(&ctx_history.stack);
				stack_clear(&ctx_syntax.stack);
				stack_clear(&ctx_alias.stack);
				return (ft_strdup(""));
			}
	
			if (add_newline)
				input = ft_strjoin_sep(input, "\n", cont_line, 6);
			else if (is_escape) {
				size_t len = ft_strlen(input);
				while (len && ft_isspace(input[len - 1])) input[--len] = '\0';
				if (input[len - 1] == '\\') input[--len] = '\0';
				input = ft_strjoin(input, cont_line, 1);
			} else
				input = ft_strjoin_sep(input, " ", cont_line, 6);
		}

		if (shell.interactive) history_add(input_hist, false);
		sfree(input_hist);
		stack_clear(&ctx_history.stack);
		stack_clear(&ctx_syntax.stack);
		stack_clear(&ctx_alias.stack);

		return (input);
	}

	#pragma endregion

#pragma region "Readfile"

	static char *readfile() {
		size_t size = 1024, bytes_read = 0;
		char *value = ft_calloc(size + 1, sizeof(char));

		while (1) {
			size_t bytes_to_read = size - bytes_read;
			ssize_t read_now = read(STDIN_FILENO, value + bytes_read, bytes_to_read);
			
			if (read_now == -1) { sfree(value); return (NULL); }
			
			bytes_read += read_now;
			value[bytes_read] = '\0';
					
			// Expand value if necessary
			if (bytes_read == size) {
				value = ft_realloc(value, size, (size * 2) + 1);
				size *= 2;
			}
			
			if (read_now == 0) break;
		}

		return (value);
	}

#pragma endregion

#pragma region "Input"

	char *get_input() {
		char *input = NULL;

		if (!shell.interactive) {
			if (!(input = readfile())) return (NULL);
		} else {
			if (!(input = readinput(prompt_PS1))) return (NULL);
		}

		if (ft_isspace_s(input)) return (input);

		return (expand_input(input));
	}

#pragma endregion
