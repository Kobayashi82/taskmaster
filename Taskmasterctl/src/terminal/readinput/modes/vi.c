/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   vi.c                                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/05 09:42:13 by vzurera-          #+#    #+#             */
/*   Updated: 2025/03/14 13:09:31 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "libft.h"
	#include "terminal/terminal.h"
	#include "terminal/print.h"
	#include "terminal/readinput/termcaps.h"
	#include "terminal/readinput/readinput.h"
	#include "terminal/readinput/prompt.h"
	#include "terminal/readinput/history.h"
	#include "terminal/signals.h"
	#include "hashes/variables.h"
	#include "main/shell.h"
	#include "main/options.h"
	#include "utils/paths.h"

	#include <sys/wait.h>

#pragma endregion

#pragma region "Variables"

	enum e_vi_mode { INSERT, EDIT };
	enum e_mode { CURSOR, AFTER_CURSOR, FIRST, LAST };

	static bool	number_mode, replacement_mode, replacement_char;
	static char	n[7], last_cmd, last_char[7], *clipboard, *tmp_line;
	static int	rep_n;

	static void	insert_mode(int mode);
	static int	get_n();
	static void home();

#pragma endregion

#pragma region "Input"

	#pragma region "Insert"

		#pragma region "EOF"							("CTRL + D")

			static int ctrl_d() {
				if (buffer.c == 4) {
					if (!buffer.length) {
						sfree(buffer.value); buffer.value = NULL;
						write(STDOUT_FILENO, "\r\n", 2);
					} else {
						buffer.value[buffer.length] = '\0';
						write(STDOUT_FILENO, "\r\n", 2);
					}
					return (1);
				} return (0);
			}

		#pragma endregion

		#pragma region "SIG_INT"						("CTRL + C")

			static int ctrl_c() {
				if (buffer.c == 3) {
					buffer.value[0] = '\0'; buffer.position = 0; buffer.length = 0;

					if (options.hide_ctrl_chars)	write(STDOUT_FILENO, "\r\n", 2);
					else							write(STDOUT_FILENO, "^C\r\n", 4);

					shell.exit_code = 130; nsignal = 2;
					return (1);
				} return (0);
			}

		#pragma endregion

		#pragma region "NewLine"						("CTRL + J, Enter")

			static int enter() {
				if (buffer.c == '\r' || buffer.c == '\n') {	
					buffer.value[buffer.length] = '\0';

					write(STDOUT_FILENO, "\r\n", 2);

					return (1);
				} return (0);
			}

		#pragma endregion

		#pragma region "BackSpace"						("BackSpace, CTRL + U")

			#pragma region "Char"						("BackSpace")

				static void backspace(bool push) {
					if (!buffer.length || !buffer.position || buffer.position > buffer.length) { beep(); return; }

					if (buffer.position > 0) {
						if (push) undo_push(false);
						
						size_t back_pos = 1;
						while (buffer.position - back_pos > 0 && (buffer.value[buffer.position - back_pos] & 0xC0) == 0x80) back_pos++;
						int c_width = char_width(buffer.position - back_pos, buffer.value);
						if (buffer.position < buffer.length) ft_memmove(&buffer.value[buffer.position - back_pos], &buffer.value[buffer.position], buffer.length - buffer.position);
						buffer.position -= back_pos; buffer.length -= back_pos;
						buffer.value[buffer.length] = '\0';

						cursor_left(c_width);
						write_value(STDOUT_FILENO, &buffer.value[buffer.position], buffer.length - buffer.position);
						write_value(STDOUT_FILENO, "  ", c_width); cursor_left(c_width);

						cursor_move(buffer.length, buffer.position);
					}
				}

			#pragma endregion

			#pragma region "Start"						("CTRL + U")

				static void backspace_start() {
					if (!buffer.length || !buffer.position || buffer.position > buffer.length) return;
					
					undo_push(false);

					int total_chars = chars_width(buffer.position, 0, buffer.value);

					ft_memmove(&buffer.value[0], &buffer.value[buffer.position], buffer.length - buffer.position);
					buffer.length -= buffer.position; buffer.position = buffer.length;
					buffer.value[buffer.length] = '\0';

					cursor_left(total_chars);
					write_value(STDOUT_FILENO, buffer.value, buffer.length);
					if (total_chars) {
						int tmp = total_chars;
						while (tmp--) write_value(STDOUT_FILENO, " ", 1);
						cursor_left(total_chars);
						home();
					}
				}

			#pragma endregion

		#pragma endregion

		#pragma region "Delete"							("Del, CTRL + Del, C, S, D")

			#pragma region "Char"						("Del")

				static void delete_char(bool push) {
					if (buffer.position < buffer.length) {

						if (push) undo_push(false);

						size_t back_pos = 1;
						while (buffer.position + back_pos < buffer.length && (buffer.value[buffer.position + back_pos] & 0xC0) == 0x80) back_pos++;
				
						ft_memmove(&buffer.value[buffer.position], &buffer.value[buffer.position + back_pos], buffer.length - (buffer.position + back_pos));
						buffer.length -= back_pos;

						write_value(STDOUT_FILENO, &buffer.value[buffer.position], buffer.length - buffer.position);
						write_value(STDOUT_FILENO, "  ", 2); cursor_left(2);
						cursor_move(buffer.length, buffer.position);

						if (vi_mode == EDIT && buffer.position == buffer.length && push) {
							if (buffer.position) {
								do { (buffer.position)--; } while (buffer.position > 0 && (buffer.value[buffer.position] & 0xC0) == 0x80);
								cursor_left(0);
							}
						}
					} else if (push) beep();
				}

			#pragma endregion

			#pragma region "Word"						("CTRL + Del")

				static void delete_word() {
					if (buffer.position >= buffer.length) return;

					undo_push(false);

					size_t end_pos = buffer.position;
				
					while (end_pos < buffer.length && (ft_isspace(buffer.value[end_pos]) || ft_ispunct(buffer.value[end_pos])))
						end_pos++;
					while (end_pos < buffer.length && !ft_isspace(buffer.value[end_pos]) && !ft_ispunct(buffer.value[end_pos]))
						end_pos += char_size(buffer.value[end_pos]);
				
					size_t delete_len = end_pos - buffer.position;
				
					if (delete_len > 0) {
						ft_memmove(&buffer.value[buffer.position], &buffer.value[end_pos], buffer.length - end_pos + 1);
						buffer.length -= delete_len;
				
						write_value(STDOUT_FILENO, &buffer.value[buffer.position], buffer.length - buffer.position);
						for (size_t i = 0; i < delete_len; i++) write_value(STDOUT_FILENO, " ", 1);
						cursor_left(delete_len);
						cursor_move(buffer.length, buffer.position);

					}		
					
					if (vi_mode == EDIT && buffer.position == buffer.length) {
						if (buffer.position) {
							do { (buffer.position)--; } while (buffer.position > 0 && (buffer.value[buffer.position] & 0xC0) == 0x80);
							cursor_left(0);
						}
					}
				}

			#pragma endregion

			#pragma region "End"						("C, S, D")

				static void delete_end(bool push) {
					if (!buffer.length || buffer.position > buffer.length) return;
					
					if (push) undo_push(false);

					int total_chars = chars_width(buffer.position, buffer.length, buffer.value);

					if (buffer.position < buffer.length) {
						ft_memset(&buffer.value[buffer.position], 0, buffer.length - buffer.position);
						buffer.length -= buffer.length - buffer.position;
					}

					if (total_chars) {
						int tmp = total_chars;
						while (tmp--) write_value(STDOUT_FILENO, " ", 1);
						cursor_left(total_chars);
						
						if (vi_mode == EDIT && buffer.position && buffer.position == buffer.length && push) {
							do { (buffer.position)--; } while (buffer.position > 0 && (buffer.value[buffer.position] & 0xC0) == 0x80);
							cursor_left(0);
						}
					}
				}

			#pragma endregion

		#pragma endregion

		#pragma region "Navigation"						("Home, End, Up, Down, Left, CTRL + Left, Right, CTRL + Right")

			#pragma region "Home"						("Home")

				static void home() {
					if (!buffer.length || buffer.position > buffer.length) return;

					while (buffer.position > 0) {
						do { (buffer.position)--; } while (buffer.position > 0 && (buffer.value[buffer.position] & 0xC0) == 0x80);
						cursor_left(0);
					}
				}

			#pragma endregion

			#pragma region "End"						("End")

				static void end() {
					if (!buffer.length || buffer.position > buffer.length) return;

					size_t length = buffer.length;
					if (vi_mode == EDIT) length = char_prev(buffer.length, buffer.value);

					while (buffer.position < length) {
						cursor_right(0);
						do { (buffer.position)++; } while (buffer.position < length && (buffer.value[buffer.position] & 0xC0) == 0x80);
					}
				}

			#pragma endregion

			#pragma region "Arrow Up"					("Up")

				static void arrow_up() {
					int number = get_n();
					if (!vi_mode) number = 1;

					int hist_pos = history_get_pos() - (number - 1);
					if (hist_pos < 0) hist_pos = 0;
					if (number > 1) history_set_pos(hist_pos);
					if (!history_length()) { beep(); return; }
					char *new_line = history_prev();

					if (!new_line) { beep(); return; }
					if (!tmp_line) tmp_line = ft_substr(buffer.value, 0, buffer.length);

					home(); delete_end(false);
					while (ft_strlen(new_line) >= buffer.size) {
						buffer.value = ft_realloc(buffer.value, buffer.size, buffer.size * 2);
						buffer.size *= 2;
					}
					ft_strcpy(buffer.value, new_line);
					buffer.length = ft_strlen(buffer.value);
					buffer.position = buffer.length;
					write_value(STDOUT_FILENO, buffer.value, buffer.length);

					if (vi_mode) {
						home();
						buffer.position = 0;
					}
				}

			#pragma endregion

			#pragma region "Arrow Down"					("Down")

				static void arrow_down() {
					int number = get_n();
					if (!vi_mode) number = 1;

					int hist_pos = history_get_pos() + (number - 1);
					if (number > 1) history_set_pos(hist_pos);
					if (!history_length()) { beep(); return; }

					char *new_line = history_next();
					bool free_line = false;

					if (!tmp_line && history_get_pos() == history_length() - 1) { beep(); return; }
					if (!new_line && history_get_pos() == history_length() - 1) {
						new_line = tmp_line;
						tmp_line = NULL;
						free_line = true;
					}

					home(); delete_end(false);
					while (ft_strlen(new_line) >= buffer.size) {
						buffer.value = ft_realloc(buffer.value, buffer.size, buffer.size * 2);
						buffer.size *= 2;
					}
					ft_strcpy(buffer.value, new_line);
					buffer.length = ft_strlen(buffer.value);
					buffer.position = buffer.length;
					write_value(STDOUT_FILENO, buffer.value, buffer.length);

					if (vi_mode) {
						home();
						buffer.position = 0;
					}

					if (free_line && new_line) sfree(new_line);
				}

			#pragma endregion

			#pragma region "Arrow Left"					("Left, CTRL + Left")

				// CURSOR MOVEMENT (CTRL + Left)
				//
				// Consists of a sequence of alphanumeric characters.
				// Any non-alphanumeric character is considered a delimiter or space.
				// An empty line is also considered to be a word.

				static void arrow_left() {
					int number = get_n();
					if (!vi_mode) number = 1;

					while (number--) {
						if (!buffer.ALT && !buffer.SHIFT && buffer.position > 0) {
							if (buffer.CTRL) {
								while (buffer.position > 0 && (ft_isspace(buffer.value[buffer.position - 1]) || ft_ispunct(buffer.value[buffer.position - 1]))) {
									cursor_left(0); (buffer.position)--;
								}
								while (buffer.position > 0 && !ft_isspace(buffer.value[buffer.position - 1]) && !ft_ispunct(buffer.value[buffer.position - 1])) {
									do { (buffer.position)--; } while (buffer.position > 0 && (buffer.value[buffer.position] & 0xC0) == 0x80);
									cursor_left(0);
								}
							} else if (buffer.position) {
								do { (buffer.position)--; } while (buffer.position > 0 && (buffer.value[buffer.position] & 0xC0) == 0x80);
								cursor_left(0);
							} else beep();
						} else beep();
					}
				}

			#pragma endregion

			#pragma region "Arrow Right"				("Right, CTRL + Right")

				// CURSOR MOVEMENT (CTRL + Right)
				//
				// Consists of a sequence of alphanumeric characters.
				// Any non-alphanumeric character is considered a delimiter or space.
				// An empty line is also considered to be a word.

				static void arrow_right() {
					size_t length = buffer.length;
					if (vi_mode == EDIT) length = char_prev(buffer.length, buffer.value);

					int number = get_n();
					if (!vi_mode) number = 1;

					while (number--) {
						if (!buffer.ALT && !buffer.SHIFT && buffer.position < length) {
							if (buffer.CTRL) {
								while (buffer.position < length && (ft_isspace(buffer.value[buffer.position]) || ft_ispunct(buffer.value[buffer.position]))) {
									cursor_right(0); (buffer.position)++;
								}
								while (buffer.position < length && !ft_isspace(buffer.value[buffer.position]) && !ft_ispunct(buffer.value[buffer.position])) {
									cursor_right(0);
									do { (buffer.position)++; } while (buffer.position < length && (buffer.value[buffer.position] & 0xC0) == 0x80);
								}
							} else {
								if (buffer.position < length) {
									cursor_right(0);
									do { (buffer.position)++; } while (buffer.position < length && (buffer.value[buffer.position] & 0xC0) == 0x80);
								} else beep();
							}
						} else beep();
					}
				}

			#pragma endregion

		#pragma endregion

		#pragma region "Char"

			static int print_char() {
				if (vi_mode && !replacement_char && !replacement_mode) return (1);

				size_t c_size = char_size(buffer.c);

				char new_char[c_size + 1];
				new_char[0] = buffer.c;
				for (size_t i = 1; i < c_size; i++) {
					ssize_t readed = read(STDIN_FILENO, &new_char[i], 1);
					if (readed != 1) return (1);
				}			
				new_char[c_size] = '\0';

				//	Ignore multi-space chars
				if (!options.multiwidth_chars && char_width(0, new_char) > 1) return (1);

				undo_push(true);

				// Expand buffer if necessary
				if (buffer.position + c_size >= buffer.size - 1) {
					buffer.value = ft_realloc(buffer.value, buffer.size, buffer.size * 2);
					buffer.size *= 2;
				}

				// Replacement Char/Mode
				do {
					size_t prev_pos = buffer.position;
					if ((replacement_char || replacement_mode) && buffer.position < buffer.length) {
						size_t back_pos = 1;
						while (buffer.position + back_pos < buffer.length && (buffer.value[buffer.position + back_pos] & 0xC0) == 0x80) back_pos++;

						ft_memmove(&buffer.value[buffer.position], &buffer.value[buffer.position + back_pos], buffer.length - (buffer.position + back_pos));
						buffer.length -= back_pos;
					}

					if (buffer.position < buffer.length) ft_memmove(&buffer.value[buffer.position + c_size], &buffer.value[buffer.position], buffer.length - buffer.position);

					// Insert all bytes of the character into the buffer
					for (size_t i = 0; i < c_size; i++) buffer.value[buffer.position++] = new_char[i];
					buffer.length += c_size;
					buffer.value[buffer.length] = '\0';

					write_value(STDOUT_FILENO, &buffer.value[buffer.position - c_size], buffer.length - (buffer.position - c_size));

					// Replacement Char/Mode
					if (replacement_char || replacement_mode) {
						//int c_width = char_width(0, new_char);
						write_value(STDOUT_FILENO, "  ", 2); cursor_left(2);
						if (replacement_char && rep_n == 1) buffer.position = prev_pos;
					}
					
					cursor_move(buffer.length, buffer.position);
				} while (replacement_char && --rep_n && buffer.position < buffer.length);

				if (replacement_char && buffer.position == buffer.length) {
					do { (buffer.position)--; } while (buffer.position > 0 && (buffer.value[buffer.position] & 0xC0) == 0x80);
					cursor_left(0);
				}

				return (1);
			}

		#pragma endregion

	#pragma endregion

	#pragma region "Edit"

		#pragma region "Insert Mode"					("i, I, a, A, c, s, S")

			static void insert_mode(int mode) {
				vi_mode = INSERT;
				last_cmd = 0;
				ft_memset(last_char, 0, 7);

				switch (mode) {
					case CURSOR: break;
					case AFTER_CURSOR: {
						arrow_right();
						break;
					}
					case FIRST:	{ home();	break; }
					case LAST:	{ end();	break; }
				}
			}

		#pragma endregion

		#pragma region "Navigation"						("b, w, e, B, W, E, f, F, t, T, |, ^, ;, ,")

			#pragma region "Word"						("b, w, e")

				// WORD
				//
				// Consists of a sequence of letters, digits and underscores, or a sequence of other non-blank characters, separated with white space.
				// An empty line is also considered to be a word.

				#pragma region "Left Start Word"		("b")

					static void left_start_word() {
						int number = get_n();
						if (!vi_mode) number = 1;

						if (buffer.position == 0) { beep(); return; }
						while (number-- && buffer.position > 0) {
							size_t pos = buffer.position;
							do { pos--; } while (pos > 0 && (buffer.value[pos] & 0xC0) == 0x80);

							while (ft_isspace(buffer.value[pos])) {
								cursor_left(1);
								buffer.position = pos;
								if (pos == 0) return;
								do { pos--; } while (pos > 0 && (buffer.value[pos] & 0xC0) == 0x80);
							}

							bool isalpha = (ft_isalnum(buffer.value[pos]) || buffer.value[pos] == '_');
							if (isalpha) {
								while ((ft_isalnum(buffer.value[pos]) || buffer.value[pos] == '_') && !ft_isspace(buffer.value[pos])) {
									cursor_left(char_width(pos, buffer.value));
									buffer.position = pos;
									if (pos == 0) return;
									do { pos--; } while (pos > 0 && (buffer.value[pos] & 0xC0) == 0x80);
								}
							} else {
								while (!ft_isalnum(buffer.value[pos]) && buffer.value[pos] != '_' && !ft_isspace(buffer.value[pos])) {
									cursor_left(char_width(pos, buffer.value));
									buffer.position = pos;
									if (pos == 0) return;
									do { pos--; } while (pos > 0 && (buffer.value[pos] & 0xC0) == 0x80);
								}
							}
						}
					}

				#pragma endregion

				#pragma region "Right Start Word"		("w")

					static void right_start_word() {
						int number = get_n();
						if (!vi_mode) number = 1;

						size_t length = buffer.length;
						if (vi_mode == EDIT) length = char_prev(buffer.length, buffer.value);

						if (buffer.position >= length) { beep(); return; }
						while (number-- && buffer.position < length) {
							while (ft_isspace(buffer.value[buffer.position])) {
								cursor_right(1);
								buffer.position++;
							}

							size_t pos;
							bool isalpha = (ft_isalnum(buffer.value[buffer.position]) || buffer.value[buffer.position] == '_');
							if (isalpha) {
								while ((ft_isalnum(buffer.value[buffer.position]) || buffer.value[buffer.position] == '_') && !ft_isspace(buffer.value[buffer.position])) {
									arrow_right();
									pos = buffer.position;
									do { pos++; } while (pos > 0 && (buffer.value[pos] & 0xC0) == 0x80);
									if (pos == buffer.length) break;
								}
							} else {
								while (!ft_isalnum(buffer.value[buffer.position]) && buffer.value[buffer.position] != '_' && !ft_isspace(buffer.value[buffer.position])) {
									arrow_right();
									pos = buffer.position;
									do { pos++; } while (pos > 0 && (buffer.value[pos] & 0xC0) == 0x80);
									if (pos == buffer.length) break;
								}
							}

							while (ft_isspace(buffer.value[buffer.position])) {
								cursor_right(1);
								buffer.position++;
							}
							if (pos == buffer.length) break;
						}
					}

				#pragma endregion

				#pragma region "Left End Word"			("e")

					static void right_end_word() {
						int number = get_n();
						if (!vi_mode) number = 1;

						size_t length = buffer.length;
						if (vi_mode == EDIT) length = char_prev(buffer.length, buffer.value);

						if (buffer.position >= length) { beep(); return; }
						while (number-- && buffer.position < length) {
							arrow_right();
							while (ft_isspace(buffer.value[buffer.position])) {
								cursor_right(1);
								buffer.position++;
							}

							size_t pos;
							bool isalpha = (ft_isalnum(buffer.value[buffer.position]) || buffer.value[buffer.position] == '_');
							if (isalpha) {
								while ((ft_isalnum(buffer.value[buffer.position]) || buffer.value[buffer.position] == '_') && !ft_isspace(buffer.value[buffer.position])) {
									arrow_right();
									pos = buffer.position;
									do { pos++; } while (pos > 0 && (buffer.value[pos] & 0xC0) == 0x80);
									if (pos == buffer.length) break;
								}
							} else {
								while (!ft_isalnum(buffer.value[buffer.position]) && buffer.value[buffer.position] != '_' && !ft_isspace(buffer.value[buffer.position])) {
									arrow_right();
									pos = buffer.position;
									do { pos++; } while (pos > 0 && (buffer.value[pos] & 0xC0) == 0x80);
									if (pos == buffer.length) break;
								}
							}
							if (pos == buffer.length) break;
							arrow_left();
						}
					}

				#pragma endregion

			#pragma endregion

			#pragma region "BigWord"					("B, W, E")

				// BIGWORD
				//
				// Consists of a sequence of non-blank characters, separated with white space.
				// An empty line is also considered to be a bigword.

				#pragma region "Left Start BigWord"		("B")

					static void left_start_bigword() {
						int number = get_n();
						if (!vi_mode) number = 1;

						if (buffer.position == 0) { beep(); return; }
						while (number-- && buffer.position > 0) {
							size_t pos = buffer.position;
							do { pos--; } while (pos > 0 && (buffer.value[pos] & 0xC0) == 0x80);

							while (ft_isspace(buffer.value[pos])) {
								cursor_left(1);
								buffer.position = pos;
								if (pos == 0) return;
								do { pos--; } while (pos > 0 && (buffer.value[pos] & 0xC0) == 0x80);
							}

							while (!ft_isspace(buffer.value[pos])) {
								cursor_left(char_width(pos, buffer.value));
								buffer.position = pos;
								if (pos == 0) return;
								do { pos--; } while (pos > 0 && (buffer.value[pos] & 0xC0) == 0x80);
							}
						}
					}

				#pragma endregion

				#pragma region "Right Start BigWord"	("W")

					static void right_start_bigword() {
						int number = get_n();
						if (!vi_mode) number = 1;

						size_t length = buffer.length;
						if (vi_mode == EDIT) length = char_prev(buffer.length, buffer.value);

						if (buffer.position >= length) { beep(); return; }
						while (number-- && buffer.position < length) {
							while (ft_isspace(buffer.value[buffer.position])) {
								cursor_right(1);
								buffer.position++;
							}

							size_t pos;
							while (!ft_isspace(buffer.value[buffer.position])) {
								arrow_right();
								pos = buffer.position;
								do { pos++; } while (pos > 0 && (buffer.value[pos] & 0xC0) == 0x80);
								if (pos == buffer.length) break;
							}

							while (ft_isspace(buffer.value[buffer.position])) {
								cursor_right(1);
								buffer.position++;
							}
							if (pos == buffer.length) break;
						}
					}

				#pragma endregion

				#pragma region "Right End BigWord"		("E")

					static void right_end_bigword() {
						int number = get_n();
						if (!vi_mode) number = 1;

						size_t length = buffer.length;
						if (vi_mode == EDIT) length = char_prev(buffer.length, buffer.value);

						if (buffer.position >= length) { beep(); return; }
						while (number-- && buffer.position < length) {
							arrow_right();
							while (ft_isspace(buffer.value[buffer.position])) {
								cursor_right(1);
								buffer.position++;
							}

							size_t pos;
							while (!ft_isspace(buffer.value[buffer.position])) {
								arrow_right();
								pos = buffer.position;
								do { pos++; } while (pos > 0 && (buffer.value[pos] & 0xC0) == 0x80);
								if (pos == buffer.length) break;
							}
							if (pos == buffer.length) break;
							arrow_left();
						}
					}

				#pragma endregion

			#pragma endregion

			#pragma region "Go To"						("f, F, t, T, |, ^")

				#pragma region "Char"					("f, F, t, T")

					static void goto_char(char cmd) {
						char c[7];
						ft_memset(c, 0, 7);

						if (!cmd) {
							if (read(STDIN_FILENO, c, 7) < 1) return;
							ft_memcpy(last_char, c, 7);
							last_cmd = cmd = buffer.c;
						} else ft_memcpy(c, last_char, 7);

						int number = get_n();
						size_t last_match = buffer.position;

						if (cmd == 'F' || cmd == 'T') {
							for (size_t i = buffer.position - 1; i != (size_t)-1; --i) {
								if (!ft_strncmp(&buffer.value[i], c, ft_strlen(c))) {
									while (buffer.position > i) arrow_left();
									if (cmd == 'T') arrow_right();
									last_match = buffer.position;
									if (!--number) return;
								}
							}
						} else {
							for (size_t i = buffer.position + 1; i < buffer.length; ++i) {
								if (!ft_strncmp(&buffer.value[i], c, ft_strlen(c))) {
									while (buffer.position < i) arrow_right();
									if (cmd == 't') arrow_left();
									last_match = buffer.position;
									if (--number == 0) return;
								}
							}
						}

						if (buffer.position == last_match) beep();
						while (buffer.position > last_match) arrow_left();
						while (buffer.position < last_match) arrow_right();
					}

				#pragma endregion

				#pragma region "Position"				("|")

					static void goto_position() {
						int number = get_n();
						
						home();
						while (--number) arrow_right();
					}

				#pragma endregion

				#pragma region "No IsSpace"				("^")

					static void goto_no_isspace() {
						home();
						while (buffer.position < char_prev(buffer.length, buffer.value) && ft_isspace(buffer.value[buffer.position]))
							arrow_right();
					}

				#pragma endregion

	#pragma endregion

			#pragma region "Repeat CMD"					(";, ,")

				static void repeat_cmd(bool reverse) {
					if (!last_cmd || !last_char[0]) return;
					
					if (reverse) {
						if (last_cmd == 'f')	goto_char('F');
						if (last_cmd == 'F')	goto_char('f');
						if (last_cmd == 't')	goto_char('T');
						if (last_cmd == 'T')	goto_char('t');					
					} else						goto_char(last_cmd);
				}

			#pragma endregion

		#pragma endregion

		#pragma region "Swap"							("CTRL + T")

			#pragma region "Char"						("CTRL + T")

				static void swap_char() {
					if (buffer.position == 0 || chars_width(0, buffer.length, buffer.value) < 2) { beep(); return; }
					if (buffer.position > 0) {
						undo_push(false);
						char temp[8];
						if (buffer.position < buffer.length) {
							size_t back_pos1 = 1, back_pos2 = 1;
							while (buffer.position - back_pos1 > 0 && (buffer.value[buffer.position - back_pos1] & 0xC0) == 0x80) back_pos1++;
							while (buffer.position + back_pos2 < buffer.length && (buffer.value[buffer.position + back_pos2] & 0xC0) == 0x80) back_pos2++;

							if (back_pos1 > 8 || back_pos2 > 8) return;

							ft_memcpy(temp, &buffer.value[buffer.position - back_pos1], back_pos1);
							ft_memmove(&buffer.value[buffer.position - back_pos1], &buffer.value[buffer.position], back_pos2);
							buffer.position -= back_pos1; buffer.position += back_pos2;
							ft_memmove(&buffer.value[buffer.position], temp, back_pos1);

							cursor_left(char_width(0, temp));
							write_value(STDOUT_FILENO, &buffer.value[buffer.position - back_pos2], back_pos1 + back_pos2);
							buffer.position += back_pos1;
						} else {
							size_t back_pos1 = 1, back_pos2 = 1;
							while (buffer.position - back_pos1 > 0 && (buffer.value[buffer.position - back_pos1] & 0xC0) == 0x80) back_pos1++;
							if (back_pos1 > 8) return;
							buffer.position -= back_pos1;
							while (buffer.position - back_pos2 > 0 && (buffer.value[buffer.position - back_pos2] & 0xC0) == 0x80) back_pos2++;

							ft_memcpy(temp, &buffer.value[buffer.position - back_pos2], back_pos2);
							ft_memmove(&buffer.value[buffer.position - back_pos2], &buffer.value[buffer.position], back_pos1);
							buffer.position -= back_pos2; buffer.position += back_pos1;
							ft_memmove(&buffer.value[buffer.position], temp, back_pos2);

							cursor_left(char_width(0, temp));
							cursor_left(char_width(buffer.position - back_pos1, buffer.value));

							write_value(STDOUT_FILENO, &buffer.value[buffer.position - back_pos1], back_pos1 + back_pos2);
							buffer.position += back_pos2;
						}
						if (vi_mode == EDIT && buffer.position > char_prev(buffer.length, buffer.value)) arrow_left();
					}
				}

			#pragma endregion

		#pragma endregion

		#pragma region "Delete"							("X, s, x, c, d")

			#pragma region "[N] BackSpace"				("X")

				static void n_backspace() {
					int number = get_n();

					undo_push(false);

					while (number--) backspace(false);
				}

			#pragma endregion

			#pragma region "[N] Delete Char"			("s, x")

				static void n_delete_char() {
					if (buffer.position == buffer.length) { beep(); return; }
					int number = get_n();

					undo_push(false);

					while (number--) delete_char(false);

					if (vi_mode == EDIT && buffer.position == buffer.length) {
						if (buffer.position) {
							do { (buffer.position)--; } while (buffer.position > 0 && (buffer.value[buffer.position] & 0xC0) == 0x80);
							cursor_left(0);
						}
					}
				}

			#pragma endregion

			#pragma region "[N] Delete To"				("c, d")

				static void n_delete_to() {

				}

			#pragma endregion

		#pragma endregion

		#pragma region "Copy"							("y, Y")

			static void copy(bool to_end) {
				if (to_end) {
					if (clipboard) sfree(clipboard);
					clipboard = ft_strdup(&buffer.value[buffer.position]);
				}
			}

		#pragma endregion

		#pragma region "Paste"							("p, P")

			static void paste(bool reverse) {
				if (!clipboard || !*clipboard) return;

				undo_push(false);

				int number = get_n();

				while (number--) {
					if (reverse)	arrow_left();
					else			{ insert_mode(CURSOR); arrow_right(); }

					// Expand buffer if necessary
					while (buffer.length + ft_strlen(clipboard) >= buffer.size - 1) {
						buffer.value = ft_realloc(buffer.value, buffer.size, buffer.size * 2);
						buffer.size *= 2;
					}

					if (buffer.position < buffer.length) ft_memmove(&buffer.value[buffer.position + ft_strlen(clipboard)], &buffer.value[buffer.position], buffer.length - buffer.position);

					// Insert all bytes of the character into the buffer
					ft_memcpy(&buffer.value[buffer.position], clipboard, ft_strlen(clipboard));

					buffer.length += ft_strlen(clipboard);
					buffer.value[buffer.length] = '\0';

					write(STDOUT_FILENO, &buffer.value[buffer.position], buffer.length - buffer.position);
					buffer.position += ft_strlen(clipboard);

					// Adjust the cursor position in the terminal
					size_t move_back = 0;
					for (size_t i = buffer.position; i < buffer.length; ) {
						if (char_width(i, buffer.value) == 2) move_back++;
						if ((unsigned char)buffer.value[i] >= 0xC0) {
							if ((unsigned char)buffer.value[i] >= 0xF0)			i += 4;	// 4 bytes
							else if ((unsigned char)buffer.value[i] >= 0xE0)	i += 3;	// 3 bytes
							else												i += 2;	// 2 bytes
						} else													i++;	// 1 byte
						move_back++;
					}
					while (move_back--) cursor_left(1);

					if (!reverse) {
						vi_mode = EDIT;
						if (buffer.position) {
							do { (buffer.position)--; } while (buffer.position > 0 && (buffer.value[buffer.position] & 0xC0) == 0x80);
							cursor_left(0);
						}
					}
				}
			}

		#pragma endregion

		#pragma region "Undo"							("u, U")

			static void n_undo(bool all) {			
				int len = chars_width(0, buffer.position, buffer.value);
				if (len > 0) cursor_left(len);
				
				len = chars_width(0, buffer.length, buffer.value);
				for (int i = len; i > 0; --i) write_value(STDOUT_FILENO, " ", 1);
				if (len > 0) cursor_left(len);
				
				if (all) undo_all();
				else {
					int number = get_n();
					while (number--) undo_pop();
				}

				write_value(STDOUT_FILENO, buffer.value, buffer.length);

				len = chars_width(buffer.position, buffer.length, buffer.value);
				if (len > 0) cursor_left(len);
			}

		#pragma endregion

		#pragma region "Comment"						("#")

			static int comment() {
				if (buffer.length + 1 >= buffer.size) {
					buffer.value = ft_realloc(buffer.value, buffer.size, buffer.size * 2);
					buffer.size *= 2;
				}

				home();
				ft_memmove(&buffer.value[1], &buffer.value[0], buffer.length);
				buffer.value[0] = '#';
				buffer.length++;

				buffer.value[buffer.length] = '\0';
				write(STDOUT_FILENO, buffer.value, buffer.length);
				write(STDOUT_FILENO, "\r\n", 2);
				
				return (2);
			}

		#pragma endregion

		#pragma region "Edit Input"						("v")

			static const char *default_editor() {
				const char	*editor = variables_find_value(vars_table, "FCEDIT");
				if (!editor || !*editor) editor = variables_find_value(vars_table, "EDITOR");
				if (!editor || !*editor) editor = variables_find_value(vars_table, "VISUAL");
				if (!editor || !*editor) editor = resolve_symlink("/usr/bin/editor");
				if (!editor || !*editor) editor = "nano";
				return (editor);
			}

			static int edit_input() {
				const char *raw_editor = default_editor();
				int fd = tmp_find_fd_path(ft_mkdtemp(NULL, "input"));
				if (fd == -1) { beep(); return (1); }

				if (write(fd, buffer.value, buffer.length) == -1) {
					tmp_delete_fd(fd);
					return (1);
				} sclose(fd);

				char *editor = get_fullpath((char *)raw_editor);
				if (access(editor, X_OK) == -1) {
					sfree(editor);
					tmp_delete_fd(fd);
					beep();
					return (1);
				}

				char *tmp_file = tmp_find_path_fd(fd);

				pid_t pid = fork();
				if (pid < 0) {
					sfree(editor);
					tmp_delete_fd(fd);
					beep();
					return (1);
				} else if (pid == 0) {
					char *const args[] = { editor, tmp_find_path_fd(fd), NULL };
					char **env = variables_to_array(vars_table, EXPORTED, true);
					sclose_all();
					execve(editor, args, env);
					beep(); sexit(1);
				} else if (pid > 0) {
					int status;
					waitpid(pid, &status, 0);

					fd = sopen(tmp_file, O_RDONLY, -1);
					if (fd == -1) {
						sfree(editor);
						tmp_delete_path(tmp_file);
						beep();
						return (1);
					}

					char	*file_content = NULL;
					char	temp_buffer[1024];
					size_t	file_size = 0;
					int		readed = 0;

					while ((readed = read(fd, temp_buffer, sizeof(temp_buffer))) > 0) {
						file_content = ft_realloc(file_content, ft_strlen(file_content), file_size + readed + 1);
						ft_memcpy(file_content + file_size, temp_buffer, readed);
						file_size += readed;
					}

					if (readed < 0) {
						beep();
						unlink(tmp_file);
					} else {
						file_content[file_size] = '\0';

						if (file_size > 0 && file_content[file_size - 1] == '\n') {
							file_content[file_size - 1] = '\0';
							file_size--;
						}
						if (file_size > 0 && file_content[file_size - 1] == '\r') {
							file_content[file_size - 1] = '\0';
							file_size--;
						}

						sfree(buffer.value);
						buffer.value = file_content;
						print(STDOUT_FILENO, "\n", RESET);
						cursor_start_column();
						print(STDOUT_FILENO, buffer.value, PRINT);
					}

					sfree(editor);
					tmp_delete_path(tmp_file);

					write(STDOUT_FILENO, "\r\n", 2);
					return (2);
				} return (1);
			}

		#pragma endregion

		#pragma region "Number"							("0-9")

			#pragma region "Number Mode Off"

				static int num_mode_off() {
					number_mode = false;
					int num_len = 8 + ft_strlen(n) + chars_width(0, buffer.position, buffer.value);
					if (num_len > 0) cursor_left(num_len);
					num_len = 8 + ft_strlen(n) + chars_width(0, buffer.length, buffer.value);
					for (int i = num_len; i > 0; --i) write_value(STDOUT_FILENO, " ", 1);
					if (num_len > 0) cursor_left(num_len);
					if (term_prompt) {
						write(STDOUT_FILENO, term_prompt, ft_strlen(term_prompt));
						cursor_update(nocolor_length(term_prompt));
					}
					write_value(STDOUT_FILENO, buffer.value, buffer.length);

					num_len = chars_width(buffer.position, buffer.length, buffer.value);
					if (num_len > 0) cursor_left(num_len);

					if (buffer.c == 27) return (0);
					if (!ft_strchr("csdxXrRypPbBwWeEfFtT;,|kjhlu ", buffer.c)) {
						if (buffer.c != 3 && buffer.c != 4 && buffer.c != '\r' && buffer.c != '\n') beep();
						return (1);
					}
					return (0);
				}

			#pragma endregion

			#pragma region "Get N"

				static int get_n() {
					int number = ft_max(1, ft_atoi(n));
					number_mode = false;
					ft_memset(n, 0, 7);

					return (number);
				}

			#pragma endregion

			#pragma region "Set N"

				static void set_n() {
					if (!number_mode && buffer.c == 48) { home(); return; }					// Move to the start of the line
					if (!number_mode) ft_memset(n, 0, 7);

					int pos = ft_max(ft_strlen(n), 0);
					if (pos > 6) {
						num_mode_off();
						ft_memset(n, 0, 7);
						return;
					}
					n[pos] = buffer.c;
					if (number_mode == false) {
						number_mode = true;
						char *prompt = remove_colors(term_prompt);
						int num_len = chars_width(0, ft_strlen(prompt), prompt);
						sfree(prompt);
						num_len += chars_width(0, buffer.position, buffer.value);
						if (num_len > 0) cursor_left(num_len);
						num_len += chars_width(buffer.position, buffer.length, buffer.value);
						for (int i = num_len; i > 0; --i) write_value(STDOUT_FILENO, " ", 1);
						if (num_len > 0) cursor_left(num_len);
					} else {
						int num_len = 7 + ft_strlen(n) + chars_width(0, buffer.position, buffer.value);
						if (num_len > 0) cursor_left(num_len);
						num_len = 7 + ft_strlen(n) + chars_width(0, buffer.position, buffer.value);
						for (int i = num_len; i > 0; --i) write_value(STDOUT_FILENO, " ", 1);
						if (num_len > 0) cursor_left(num_len);
					}
					write_value(STDOUT_FILENO, "(arg: ", 6);
					write_value(STDOUT_FILENO, n, ft_strlen(n));
					write_value(STDOUT_FILENO, ") ", 2);
					write_value(STDOUT_FILENO, buffer.value, buffer.length);
					int num_len = chars_width(buffer.position, buffer.length, buffer.value);
					if (num_len > 0) cursor_left(num_len);
				}

			#pragma endregion

		#pragma endregion

	#pragma endregion

	#pragma region "Handle"

		#pragma region "Navigation"

			#pragma region "Modifiers"

				static char modifiers(char *seq) {
					int		modifier = 0;
					char	key = seq[1];
					size_t	i = 0;

					buffer.CTRL = false; buffer.ALT = false; buffer.SHIFT = false;
					if (seq[1] == '3') return (seq[1]);
					if (seq[0] == '[') { while (seq[i] && seq[i] != ';') i++;
						if (seq[i] == ';') { i++;
							while (seq[i] >= '0' && seq[i] <= '9')
								modifier = modifier * 10 + (seq[i++] - '0');
							if (seq[i]) key = seq[i];
						}
					}
					if (modifier && key) {
						if (modifier == 2)		buffer.SHIFT = true;
						if (modifier == 3)		buffer.ALT   = true;
						if (modifier == 5)		buffer.CTRL  = true;
						if (modifier == 6) {	buffer.SHIFT = true; buffer.CTRL = true; }
						if (modifier == 7) {	buffer.ALT   = true; buffer.CTRL = true; }
						if (modifier == 8) {	buffer.SHIFT = true; buffer.ALT  = true; buffer.CTRL = true; }
					}
					return (key);
				}

			#pragma endregion

			#pragma region "Cursor"

				static int cursor() {
					char seq[8];
					ft_memset(&seq, 0, sizeof(seq));
					if (buffer.c == 27) {

						fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
						int result = read(STDIN_FILENO, seq, sizeof(seq) - 1);
						fcntl(STDIN_FILENO, F_SETFL, O_SYNC);

						if (result > 0) {
							if (seq[0] == '[') { seq[1] = modifiers(seq);
								if (seq[1] == 'A') 						arrow_up();				//	Up				History next
								if (seq[1] == 'B') 						arrow_down();			//	Down			History prev
								if (seq[1] == 'D') 						arrow_left();			//	Left			Cursor left
								if (seq[1] == 'C') 						arrow_right();			//	Right			Cursor right
								if (seq[1] == 'H') 						home();					//	Home			Cursor to the start
								if (seq[1] == 'F')						end();					//	End				Cursor to the end
								if (seq[1] == '3' && seq[2] == '~')		delete_char(1);			//	Del				Delete
								if (!ft_strncmp(seq + 1, "3;5~", 4))	delete_word();			//	CTRL + Del		Delete current word
							}
						} else if (!vi_mode) {
							vi_mode = EDIT;														//	Esc				Edit mode
							if (buffer.position) {
								do { (buffer.position)--; } while (buffer.position > 0 && (buffer.value[buffer.position] & 0xC0) == 0x80);
								cursor_left(0);
							}
						} return (1);
					} return (0);
				}

			#pragma endregion

		#pragma endregion

		#pragma region "Specials"

			static int specials() {
				if (buffer.c == 127 && !vi_mode)			{ backspace(1);					}	//	[BackSpace]	Delete the previous character									(Only in insertion mode)
				else if (buffer.c == 8 && !vi_mode)			{ backspace(1);					}	//	[CTRL + H]	Delete the previous character									(Only in insertion mode)
				else if (buffer.c == 9)						{ autocomplete();				}	//	[Tab]		Auto-Complete
				else if (buffer.c == 10)					{ enter();						}	//	[CTRL + J]	Enter
				else if (buffer.c == 18)					{ history_search();				}	//	[CTRL + R]	History incremental search
				else if (buffer.c == 20)					{ swap_char();					}	//	[CTRL + T]	Swap the current character with the previous one
				else if (buffer.c == 21)					{ backspace_start();			}	//	[CTRL + U]	Backspace from cursor to the start of the line
				else if (buffer.c == 31)					{ n_undo(0);					}	//	[CTRL + _]	Undo the last change
				else if (buffer.c >= 1 && buffer.c <= 26)	{ ;								}	//	Ignore other CTRL + X commands
				else if (buffer.c >= 28 && buffer.c <= 31)	{ ;								}	//	Ignore other CTRL + X commands
				else if (vi_mode) {
					if (ft_isdigit(buffer.c))	{ set_n();									}	//	Set the repetition number for commands

					else if (buffer.c == 'i')	{ insert_mode(CURSOR);						}	//	Enter insert mode at the cursor position
					else if (buffer.c == 'I')	{ insert_mode(FIRST);						}	//	Enter insert mode at the beginning of the line
					else if (buffer.c == 'a')	{ insert_mode(AFTER_CURSOR);				}	//	Enter insert mode after the cursor position
					else if (buffer.c == 'A')	{ insert_mode(LAST);						}	//	Enter insert mode at the end of the line
					else if (buffer.c == 'c')	{ insert_mode(CURSOR); n_delete_to();		}	//- [n] Delete up to the specified position and enter insert mode				(0, ^, $ , |, , ;,, fFtTbBeEwW, c)
					else if (buffer.c == 'C')	{ insert_mode(CURSOR); delete_end(1);		}	//	Delete from cursor to the end of the line and enter insert mode
					else if (buffer.c == 's')	{ insert_mode(CURSOR); n_delete_char();		}	//	[n] Delete the current character and enter insert mode
					else if (buffer.c == 'S')	{ insert_mode(FIRST);  delete_end(1);		}	//	Delete the entire line and enter insert mode
					else if (buffer.c == 'd')	{ n_delete_to();							}	//- [n] Delete up to the specified position										(0, ^, $, |, , ;,, fFtTbBeEwW, d)
					else if (buffer.c == 'D')	{ delete_end(1);							}	//	Delete from cursor to the end of the line
					else if (buffer.c == 'x')	{ n_delete_char();							}	//	[n] Delete the current character
					else if (buffer.c == 'X')	{ n_backspace();							}	//	[n] Delete the previous character

					else if (buffer.c == 'r')	{ replacement_char = true; rep_n = get_n();	}	//	[n] Replace the current character with the specified one
					else if (buffer.c == 'R')	{ replacement_mode = true;					}	//	Enter replace mode: allows replacing characters one by one
					else if (buffer.c == 'y')	{ copy(false);								}	//-	[n] Copy up to the specified position										(0, ^, $, |, , ;,, fFtTbBeEwW, y)
					else if (buffer.c == 'Y')	{ copy(true);								}	//	Copy from the current position to the end of the line
					else if (buffer.c == 'p')	{ paste(false);								}	//	[n] Paste copied text after the cursor
					else if (buffer.c == 'P')	{ paste(true);								}	//	[n] Paste copied text before the cursor

					else if (buffer.c == 'b')	{ left_start_word();						}	//	[n] Move the cursor to the beginning of the previous word
					else if (buffer.c == 'B')	{ left_start_bigword();						}	//	[n] Move the cursor to the beginning of the previous big word
					else if (buffer.c == 'w')	{ right_start_word();						}	//	[n] Move the cursor to the beginning of the next word
					else if (buffer.c == 'W')	{ right_start_bigword();					}	//	[n] Move the cursor to the beginning of the next big word
					else if (buffer.c == 'e')	{ right_end_word();							}	//	[n] Move the cursor to the end of the current word
					else if (buffer.c == 'E')	{ right_end_bigword();						}	//	[n] Move the cursor to the end of the current big word

					else if (buffer.c == '^')	{ goto_no_isspace();						}	//	Move the cursor to the first non-whitespace character in the line
					else if (buffer.c == 'f')	{ goto_char(0);								}	//	[n] Move the cursor forward to the character specified
					else if (buffer.c == 'F')	{ goto_char(0);								}	//	[n] Move the cursor backward to the character specified
					else if (buffer.c == 't')	{ goto_char(0);								}	//	[n] Move the cursor forward one character before the character specified
					else if (buffer.c == 'T')	{ goto_char(0);								}	//	[n] Move the cursor backward one character after the character specified
					else if (buffer.c == ';')	{ repeat_cmd(false);						}	//	[n] Repeat the last character search command								(f, F, t, T)
					else if (buffer.c == ',')	{ repeat_cmd(true);							}	//	[n] Repeat the last character search command in reverse						(f, F, t, T)
					else if (buffer.c == '|')	{ goto_position();							}	//	[n] Move the cursor to a specific character position						(default is 1)

					else if (buffer.c == '-')	{ arrow_up();								}	//	[n] Move the cursor up
					else if (buffer.c == '+')	{ arrow_down();								}	//	[n] Move the cursor down
					else if (buffer.c == 'k')	{ arrow_up();								}	//	[n] Move the cursor up
					else if (buffer.c == 'j')	{ arrow_down();								}	//	[n] Move the cursor down
					else if (buffer.c == 'h')	{ arrow_left();								}	//	[n] Move the cursor left
					else if (buffer.c == 'l')	{ arrow_right();							}	//	[n] Move the cursor right
					else if (buffer.c == ' ')	{ arrow_right();							}	//	[n] Move the cursor right
					else if (buffer.c == '$')	{ end();									}	//	Move the cursor to the end of the line
					else if (buffer.c == 'u')	{ n_undo(0);								}	//	[n] Undo the last change
					else if (buffer.c == 'U')	{ n_undo(1);								}	//	Undo all changes
					else if (buffer.c == 'v')	{ return (edit_input());					}	//	Edit the input using the default editor and terminate the input
					else if	(buffer.c == '#')	{ return (comment());						}	//	Comment and terminate the input

					else return (0);
				} else return (0);

				return (1);
			}

		#pragma endregion

	#pragma endregion

#pragma endregion

#pragma region "Vi"

	int vi() {
		int result = 0;

		if (vi_mode && !ft_isdigit(buffer.c) && !number_mode)	ft_memset(n, 0, 7);
		if (vi_mode && !ft_isdigit(buffer.c) && number_mode) {
			if (num_mode_off()) {
				if		(ctrl_d())				result = 1;
				else if	(ctrl_c())				result = 1;
				else if	(enter())				result = 1;

				if (result && clipboard) sfree(clipboard);
				return (result);
			}
		}

		if ((replacement_char || replacement_mode)) {
			if		(ctrl_d())				result = 1;
			else if	(ctrl_c())				result = 1;
			else if	(enter())				result = 1;
			else if (ft_isprint(buffer.c)) {
				print_char();
				if (replacement_mode) return (0);
			}
			if (replacement_char) replacement_char = false;
			if (replacement_mode) { replacement_mode = false;
				if (buffer.position && !result) {
					do { (buffer.position)--; } while (buffer.position > 0 && (buffer.value[buffer.position] & 0xC0) == 0x80);
					cursor_left(0);
					if (buffer.c != 27) beep();
				}
			}

			if (result && clipboard) sfree(clipboard);
			return (result);
		} replacement_mode = false;

		if		(ctrl_d())				result = 1;
		else if	(ctrl_c())				result = 1;
		else if	(enter())				result = 1;
		else if ((result = specials()))	result = (result == 2);
		else if (cursor())				result = 0;
		else if (print_char())			result = 0;

		if (result) insert_mode(CURSOR);
		if (result && clipboard) sfree(clipboard);
		if (result && tmp_line) { sfree(tmp_line); tmp_line = NULL; }
		return (result);
	}

#pragma endregion
