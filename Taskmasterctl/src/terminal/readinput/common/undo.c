/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   undo.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/05 10:10:10 by vzurera-          #+#    #+#             */
/*   Updated: 2025/02/23 12:32:21 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "libft.h"
	#include "terminal/readinput/termcaps.h"
	#include "terminal/readinput/readinput.h"

#pragma endregion

#pragma region "Variables"

	typedef struct s_undo {
		char			*value;
		size_t			size, position, length;
		struct s_undo	*next;
	} t_undo;

	static t_undo	*stack;
	static bool		pushed;

#pragma endregion

#pragma region "Push"

	void undo_push(bool push) {
		if (push && !pushed) return;
		if (stack && !ft_strcmp(stack->value, buffer.value) && stack->length == buffer.length && stack->size == buffer.size) return;
		t_undo *new = smalloc(sizeof(t_undo));
		new->size = buffer.size;
		new->length = buffer.length;
		new->position = buffer.position;
		new->value = smalloc(buffer.size);
		ft_memcpy(new->value, buffer.value, buffer.size);
		new->next = stack;
		stack = new;
		pushed = push;
	}

#pragma endregion

#pragma region "Undo"

	void undo_pop() {
		if (!stack) { beep(); return; }
		t_undo *top = stack;

		sfree(buffer.value);
		buffer.size = top->size;
		buffer.length = top->length;
		buffer.position = top->position;
		buffer.value = smalloc(top->size);
		ft_memcpy(buffer.value, top->value, top->size);
		stack = top->next;
		sfree(top->value);
		sfree(top);
		pushed = false;
	}

#pragma endregion

#pragma region "Undo All"

	void undo_all() {
		while (stack && stack->next) {
			t_undo *tmp = stack;
			stack = stack->next;
			sfree(tmp->value);
			sfree(tmp);
		} undo_pop();
	}

#pragma endregion

#pragma region "Clear"

	void undo_clear() {
		while (stack) {
			t_undo *tmp = stack;
			stack = stack->next;
			sfree(tmp->value);
			sfree(tmp);
		}
	}

#pragma endregion
