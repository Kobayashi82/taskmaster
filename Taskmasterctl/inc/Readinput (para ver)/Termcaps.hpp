/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Termcaps.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 00:00:00 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/13 00:00:00 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>

namespace Terminal {

	class Termcaps {
	public:
		Termcaps() = delete;  // Static class
		~Termcaps() = delete;

		// Character utilities
		static size_t		getCharSize(unsigned char c);
		static size_t		getCharWidth(size_t position, const std::string& value);
		static size_t		getCharsWidth(size_t from, size_t to, const std::string& value);
		static size_t		getPrevChar(size_t position, const std::string& value);
		static size_t		getNoColorLength(const std::string& str);
		static std::string	removeColors(const std::string& str);

		// Cursor navigation
		static void			cursorUp();
		static void			cursorDown();
		static void			cursorLeft(int moves);
		static void			cursorRight(int moves);

		// Cursor position
		static void			cursorGet();
		static void			cursorSet(size_t new_row, size_t new_col);
		static void			cursorMove(size_t from, size_t to);
		static void			cursorUpdate(size_t length);
		static void			cursorStartColumn();

		// Cursor visibility
		static void			cursorHide();
		static void			cursorShow();

		// Output
		static int			writeValue(int fd, const std::string& value);
		static int			writeValue(int fd, const char* value, size_t length);

		// Terminal management
		static int			initialize();
		static void			release();
	};

}