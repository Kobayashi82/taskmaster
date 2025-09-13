/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   termcaps.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/27 13:30:40 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/13 23:11:36 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region Includes

	#include <stddef.h>

#pragma endregion

#pragma region Methods

	//	---------- UTILS -----------
	size_t	char_size(unsigned char c);
	size_t	char_width(size_t position, const char *value);
	size_t	chars_width(size_t from, size_t to, const char *value);
	size_t	char_prev(size_t position, const char *value);
	size_t	nocolor_length(const char *str);
	char	*remove_colors(const char *str);
	//	-------- NAVIGATION --------
	void	cursor_up();
	void	cursor_down();
	void	cursor_left(int moves);
	void	cursor_right(int moves);
	//	--------- POSITION ---------
	void	cursor_get();
	void	cursor_set(size_t new_row, size_t new_col);
	void	cursor_move(size_t from, size_t to);
	void	cursor_update(size_t length);
	void	cursor_start_column();
	//	-------- VISIBILITY --------
	void	cursor_hide();
	void	cursor_show();
	//	---------- WRITE -----------
	int		write_value(int fd, const char *value, size_t length);
	//	-------- INITIALIZE --------
	int		terminal_initialize();
	void	terminal_release();
	void	beep();

#pragma endregion
