/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   undo.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/05 10:10:10 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/14 12:54:37 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Readline/termcaps.hpp"
	#include "Readline/readinput.hpp"

	#include <cstring>
	#include <stdlib.h>

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
		if (stack && !strcmp(stack->value, buffer.value) && stack->length == buffer.length && stack->size == buffer.size) return;
		t_undo *new_val = (t_undo *)malloc(sizeof(t_undo));
		new_val->size = buffer.size;
		new_val->length = buffer.length;
		new_val->position = buffer.position;
		new_val->value = (char *)malloc(buffer.size);
		memcpy(new_val->value, buffer.value, buffer.size);
		new_val->next = stack;
		stack = new_val;
		pushed = push;
	}

#pragma endregion

#pragma region "Undo"

	void undo_pop() {
		if (!stack) { beep(); return; }
		t_undo *top = stack;

		free(buffer.value);
		buffer.size = top->size;
		buffer.length = top->length;
		buffer.position = top->position;
		buffer.value = (char *)malloc(top->size);
		memcpy(buffer.value, top->value, top->size);
		stack = top->next;
		free(top->value);
		free(top);
		pushed = false;
	}

#pragma endregion

#pragma region "Undo All"

	void undo_all() {
		while (stack && stack->next) {
			t_undo *tmp = stack;
			stack = stack->next;
			free(tmp->value);
			free(tmp);
		} undo_pop();
	}

#pragma endregion

#pragma region "Clear"

	void undo_clear() {
		while (stack) {
			t_undo *tmp = stack;
			stack = stack->next;
			free(tmp->value);
			free(tmp);
		}
	}

#pragma endregion
