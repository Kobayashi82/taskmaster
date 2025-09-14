/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   history_search.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/06 15:20:34 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/14 14:46:15 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Readline/termcaps.hpp"
	#include "Readline/readinput.hpp"
	#include "Readline/History.hpp"

	#include <string>
	#include <cstring>
	#include <stdlib.h>
	#include <unistd.h>
	#include <cctype>

#pragma endregion

#pragma region "Variables"

	enum		e_mode { START, FORWARD, BACKWARD };

	t_buffer		search_buffer;
	bool			hist_searching;							//	Indicates whether the terminal is in searching mode
	
	static bool		initialized;

	static size_t	original_size, original_position;
	static char		*original_buffer;

	static size_t	history_pos, old_len;
	static bool		no_match;
	static char		*match_show;

	static int		search_find(int mode);

	static bool		multiwidth_chars = false;

#pragma endregion

static size_t safe_strlen(const char *s) {
    return (s ? strlen(s) : 0);
}

#pragma region "Input"

	#pragma region "BackSpace"					("BackSpace")

		static void backspace() {
			if (!search_buffer.length || !search_buffer.position || search_buffer.position > search_buffer.length) { beep(); return; }

			if (search_buffer.position > 0) {
				// Move cursor to the beginning of the line
				int len = chars_width(0, search_buffer.position, search_buffer.value);
				if (no_match) len += 26; else len += 19;
				if (len > 0) cursor_left(len);
				
				// Clear line
				len += 3 + chars_width(search_buffer.position, search_buffer.length, search_buffer.value);
				len += old_len;
				for (int i = len; i > 0; --i) write_value(STDOUT_FILENO, " ", 1);
				if (len > 0) cursor_left(len);
				
				// Remove the character from search buffer
				size_t back_pos = 1;
				while (search_buffer.position - back_pos > 0 && (search_buffer.value[search_buffer.position - back_pos] & 0xC0) == 0x80) back_pos++;
				if (search_buffer.position < search_buffer.length)
					memmove(&search_buffer.value[search_buffer.position - back_pos], &search_buffer.value[search_buffer.position], search_buffer.length - search_buffer.position);
				search_buffer.position -= back_pos; search_buffer.length -= back_pos;
				search_buffer.value[search_buffer.length] = '\0';

				search_find(START);

				// Write prompt
				if (no_match)	write_value(STDOUT_FILENO, "(failed reverse-i-search)`", 26);
				else			write_value(STDOUT_FILENO, "(reverse-i-search)`", 19);

				// write search and match
				write_value(STDOUT_FILENO, search_buffer.value, search_buffer.length);
				write_value(STDOUT_FILENO, "': ", 3);
				write_value(STDOUT_FILENO, match_show, safe_strlen(match_show));

				// Move cursor to input position
				len = 3 + chars_width(0, safe_strlen(match_show), match_show) + chars_width(search_buffer.position, search_buffer.length, search_buffer.value);
				if (len > 0) cursor_left(len);

				old_len = chars_width(0, buffer.length, buffer.value);
				if (!no_match) { free(match_show); match_show = NULL; }
			}
		}

	#pragma endregion

	#pragma region "Char"

		static int print_char() {			
			size_t c_size = char_size(buffer.c);
			
			char *new_char = (char *)malloc(c_size + 1);
			new_char[0] = buffer.c;
			for (size_t i = 1; i < c_size; i++) read(STDIN_FILENO, &new_char[i], 1);
			new_char[c_size] = '\0';
			
			// Ignore multi-space chars
			if (!multiwidth_chars && char_width(0, new_char) > 1) return (1);

			// Expand buffer if necessary
			if (search_buffer.position + c_size >= search_buffer.size - 1) {
				search_buffer.value = (char *)realloc(search_buffer.value, search_buffer.size * 2);
				search_buffer.size *= 2;
			}
			
			// Insert all bytes of the character into the buffer
			for (size_t i = 0; i < c_size; i++) search_buffer.value[search_buffer.position++] = new_char[i];
			search_buffer.length += c_size;
			search_buffer.value[search_buffer.length] = '\0';
			
			// Move cursor to the beginning of the line
			int len = chars_width(0, search_buffer.position - c_size, search_buffer.value);
			if (no_match) len += 26; else len += 19;
			if (len > 0) cursor_left(len);
			
			// Clear line
			len += 3 + chars_width(search_buffer.position - c_size, search_buffer.length, search_buffer.value);
			len += old_len;
			for (int i = len; i > 0; --i) write_value(STDOUT_FILENO, " ", 1);
			if (len > 0) cursor_left(len);
			
			search_find(START);

			// Write prompt
			if (no_match)	write_value(STDOUT_FILENO, "(failed reverse-i-search)`", 26);
			else			write_value(STDOUT_FILENO, "(reverse-i-search)`", 19);

			// write search and match
			write_value(STDOUT_FILENO, search_buffer.value, search_buffer.length);
			write_value(STDOUT_FILENO, "': ", 3);
			write_value(STDOUT_FILENO, match_show, safe_strlen(match_show));

			// Move cursor to input position
			len = 3 + chars_width(0, safe_strlen(match_show), match_show) + chars_width(search_buffer.position, search_buffer.length, search_buffer.value);
			if (len > 0) cursor_left(len);

			old_len = chars_width(0, buffer.length, buffer.value);
			if (!no_match) { free(match_show); match_show = NULL; }
			return (1);
		}

	#pragma endregion

#pragma endregion

#pragma region "Search"

	#pragma region "Init"

		void search_init() {
			undo_push(false);
			
			no_match = false;
			old_len = chars_width(0, buffer.length, buffer.value);
			original_size = buffer.size;
			original_position = buffer.position;
			original_buffer = (char *)malloc(buffer.size);
			memcpy(original_buffer, buffer.value, buffer.size);
			
			hist_searching = true;
			initialized = true;
			search_buffer.size = 1024;
			search_buffer.position = 0, search_buffer.length = 0;
			search_buffer.value = (char *)calloc(search_buffer.size, sizeof(char));

			char *prompt = remove_colors(term_prompt);
			int len = chars_width(0, safe_strlen(prompt), prompt); free(prompt);
			len += chars_width(0, buffer.position, buffer.value);
			if (len > 0) cursor_left(len);

			len += chars_width(buffer.position, buffer.length, buffer.value);
			for (int i = len; i > 0; --i) write_value(STDOUT_FILENO, " ", 1);
			if (len > 0) cursor_left(len);

			write_value(STDOUT_FILENO, "(reverse-i-search)`", 19);
			write_value(STDOUT_FILENO, "': ", 3); cursor_left(3);
		}

	#pragma endregion

	#pragma region "Exit"

		static int search_exit() {
			hist_searching = false;
			initialized = false;

			int len = chars_width(0, search_buffer.position, search_buffer.value);
			if (no_match) len += 26; else len += 19;
			if (len > 0) cursor_left(len);

			len = 3 + chars_width(0, search_buffer.length, search_buffer.value) + old_len;
			if (no_match) len += 26; else len += 19;

			for (int i = len; i > 0; --i) write_value(STDOUT_FILENO, " ", 1);
			if (len > 0) cursor_left(len);

			if (term_prompt) {
				write(STDOUT_FILENO, term_prompt, safe_strlen(term_prompt));
				cursor_update(nocolor_length(term_prompt));
			}
			write_value(STDOUT_FILENO, buffer.value, buffer.length);

			len = chars_width(buffer.position, buffer.length, buffer.value);
			if (len > 0) cursor_left(len);

			free(search_buffer.value);
			free(original_buffer);
			free(match_show);

			undo_push(false);

			return (0);
		}

	#pragma endregion

	#pragma region "Cancel"						("CTRL + G")
			
		static int search_cancel() {
			old_len = chars_width(0, buffer.length, buffer.value);

			free(buffer.value);
			buffer.size = original_size;
			buffer.position = original_position;
			buffer.value = (char *)malloc(original_size);
			memcpy(buffer.value, original_buffer, original_size);

			return (search_exit());
		}
		
	#pragma endregion

	#pragma region "Find"

		#pragma region "Match"
			
			static int process_match(const std::string line, size_t pos, bool update) {
				no_match = false;

				std::string before = line.substr(0, pos);
				std::string match  = line.substr(pos, safe_strlen(search_buffer.value));
				std::string after  = line.substr(pos + safe_strlen(search_buffer.value));

				std::string new_match_show = before + "\033[30;47m" + match + "\033[0m" + after;
				match_show = strdup(new_match_show.c_str());

				// Expand buffer if necessary
				while (line.length() >= buffer.size - 1) {
					buffer.value = (char *)realloc(buffer.value, buffer.size * 2);
					buffer.size *= 2;
				}

				memset(buffer.value, 0, buffer.size);
				strcpy(buffer.value, line.c_str());
				buffer.length = line.length();
				buffer.position = 0;

				if (update) {
					// Clear line
					int len = 3 + old_len;
					for (int i = len; i > 0; --i) write_value(STDOUT_FILENO, " ", 1);
					if (len > 0) cursor_left(len);
					
					// write match
					write_value(STDOUT_FILENO, "': ", 3);
					write_value(STDOUT_FILENO, match_show, safe_strlen(match_show));

					// Move cursor to input position
					len = 3 + chars_width(0, safe_strlen(match_show), match_show);
					if (len > 0) cursor_left(len);

					old_len = chars_width(0, buffer.length, buffer.value);
					free(match_show); match_show = NULL;
				}

				return (1);
			}

		#pragma endregion

		#pragma region "Find"

			static int search_find(int mode) {
				if (!search_buffer.value) return (0);
				if (*search_buffer.value) {				
					size_t len = history.length();
					if (mode == FORWARD && !history_pos)				{ beep();	return (0); }
					if (mode == BACKWARD && history_pos >= len)			{ beep();	return (0); }
					if (mode == START)									history_pos = len;
					
					if (mode == BACKWARD) {
						size_t i = history_pos + 1;
						for (; i < len; ++i) {
							char *line = strdup(history.get(i).c_str());
							if (line) {
								char *match = strstr(line, search_buffer.value);
								if (match) {
									history_pos = i;
									return (process_match(line, match - line, true));
								}
							}
						}
					} else {
						size_t i = history_pos - (mode == FORWARD);
						for (; i >= 0; --i) {
							char *line = strdup(history.get(i).c_str());
							if (line) {
								char *match = strstr(line, search_buffer.value);
								if (match) {
									history_pos = i;
									return (process_match(line, match - line, (mode == FORWARD)));
								}
							}
							if (i == 0) break;
						}
					}
				}
				
				beep();
				if (mode == START) {
					free(buffer.value);
					buffer.size = original_size;
					buffer.position = original_position;
					buffer.value = (char *)malloc(original_size);
					memcpy(buffer.value, original_buffer, original_size);
					no_match = true;
				}
				return (0);
			}

		#pragma endregion

	#pragma endregion

	#pragma region "Search"

		int history_search() {
			if (!initialized) { search_init(); return (1); }

			if 		(buffer.c == 3)			{ search_cancel(); return (0); }	//	[CTRL + C]	SIG_INT
			else if	(buffer.c == 7)			return (search_cancel());			//	[CTRL + G]	Cancel search
			else if (buffer.c == 18)		search_find(FORWARD);				//	[CTRL + R]	Search up
			else if (buffer.c == 19)		search_find(BACKWARD);				//	[CTRL + S]	Search down
			else if	(buffer.c == 127)		backspace();						//	[BackSpace]	Delete the previous character
			else if (std::isprint(buffer.c))	print_char();			
			else							return (search_exit());

			return (1);
		}

	#pragma endregion

#pragma endregion
