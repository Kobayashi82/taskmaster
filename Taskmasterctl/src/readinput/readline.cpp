/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   readline.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/01 10:32:07 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/13 23:37:51 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "readinput/terminal.hpp"
	#include "readinput/termcaps.hpp"
	#include "readinput/readinput.hpp"
	#include "readinput/history.hpp"
	#include "signals.hpp"

	#include <string>
	#include <cstring>
	#include <unistd.h>
	#include <fcntl.h>

#pragma endregion

#pragma region "Variables"

	typedef struct s_word {
		size_t	start, end, len;
		char	value[1024];
	}	t_word;

	static char	*clipboard;
	static char	*tmp_line;
	static bool	hide_ctrl_chars = false;
	static bool	multiwidth_chars = false;

	static void home();

#pragma endregion

#pragma region "Input"

	#pragma region "Insert"

		#pragma region "EOF"							("CTRL + D")

			static int ctrl_d(const int n) {
				if (n <= 0 || (buffer.c == 4 && !buffer.length)) {
					if (tmp_line) { free(tmp_line); tmp_line = NULL; }
					
					free(buffer.value); buffer.value = NULL;
					write(STDOUT_FILENO, "\r\n", 2);
					return (1);
				} return (0);
			}

		#pragma endregion

		#pragma region "SIG_INT"						("CTRL + C")

			static int ctrl_c() {
				if (buffer.c == 3) {
					if (tmp_line) { free(tmp_line); tmp_line = NULL; }

					buffer.value[0] = '\0'; buffer.position = 0; buffer.length = 0;

					if (hide_ctrl_chars)	write(STDOUT_FILENO, "\r\n", 2);
					else							write(STDOUT_FILENO, "^C\r\n", 4);

					// shell.exit_code = 130; nsignal = 2;
					return (1);
				} return (0);
			}

		#pragma endregion

		#pragma region "NewLine"						("CTRL + J, Enter")

			static int enter() {
				if (buffer.c == '\r' || buffer.c == '\n') {
					if (tmp_line) { free(tmp_line); tmp_line = NULL; }

					buffer.value[buffer.length] = '\0';

					write(STDOUT_FILENO, "\r\n", 2);

					return (1);
				} return (0);
			}

		#pragma endregion

		#pragma region "BackSpace"						("BackSpace, CTRL + U")

			#pragma region "Char"						("BackSpace")

				static void backspace() {
					if (!buffer.length || !buffer.position || buffer.position > buffer.length) { beep(); return; }

					if (buffer.position > 0) {

						undo_push(false);

						size_t back_pos = 1;

						while (buffer.position - back_pos > 0 && (buffer.value[buffer.position - back_pos] & 0xC0) == 0x80) back_pos++;
						int c_width = char_width(buffer.position - back_pos, buffer.value);
						if (buffer.position < buffer.length) memmove(&buffer.value[buffer.position - back_pos], &buffer.value[buffer.position], buffer.length - buffer.position);
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
					if (!buffer.length || !buffer.position || buffer.position > buffer.length) { beep(); return; }

					undo_push(false);

					int total_chars = chars_width(buffer.position, 0, buffer.value);

					memmove(&buffer.value[0], &buffer.value[buffer.position], buffer.length - buffer.position);
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

				static void delete_char() {
					if (buffer.position < buffer.length) {
						undo_push(false);

						size_t back_pos = 1;
						while (buffer.position + back_pos < buffer.length && (buffer.value[buffer.position + back_pos] & 0xC0) == 0x80) back_pos++;

						memmove(&buffer.value[buffer.position], &buffer.value[buffer.position + back_pos], buffer.length - (buffer.position + back_pos));
						buffer.length -= back_pos;

						write_value(STDOUT_FILENO, &buffer.value[buffer.position], buffer.length - buffer.position);
						write_value(STDOUT_FILENO, "  ", 2); cursor_left(2);
						cursor_move(buffer.length, buffer.position);
					} else beep();
				}

			#pragma endregion

			#pragma region "Word"						("CTRL + Del")

				static void delete_word() {
					if (buffer.position >= buffer.length) return;
					
					undo_push(false);

					size_t end_pos = buffer.position;
				
					while (end_pos < buffer.length && (std::isspace(buffer.value[end_pos]) || std::ispunct(buffer.value[end_pos])))
						end_pos++;
					while (end_pos < buffer.length && !std::isspace(buffer.value[end_pos]) && !std::ispunct(buffer.value[end_pos]))
						end_pos += char_size(buffer.value[end_pos]);
				
					size_t delete_len = end_pos - buffer.position;
				
					if (delete_len > 0) {
						memmove(&buffer.value[buffer.position], &buffer.value[end_pos], buffer.length - end_pos + 1);
						buffer.length -= delete_len;
				
						write_value(STDOUT_FILENO, &buffer.value[buffer.position], buffer.length - buffer.position);
						for (size_t i = 0; i < delete_len; i++) write_value(STDOUT_FILENO, " ", 1);
						cursor_left(delete_len);
						cursor_move(buffer.length, buffer.position);
					}
				}

			#pragma endregion

			#pragma region "End"						("C, S, D")

				static void delete_end(bool push) {
					if (!buffer.length || buffer.position > buffer.length) return;

					if (push) undo_push(false);

					int total_chars = chars_width(buffer.position, buffer.length, buffer.value);

					if (buffer.position < buffer.length) {
						memset(&buffer.value[buffer.position], 0, buffer.length - buffer.position);
						buffer.length -= buffer.length - buffer.position;
					}

					if (total_chars) {
						int tmp = total_chars;
						while (tmp--) write_value(STDOUT_FILENO, " ", 1);
						cursor_left(total_chars);
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

					while (buffer.position < buffer.length) {
						cursor_right(0);
						do { (buffer.position)++; } while (buffer.position < buffer.length && (buffer.value[buffer.position] & 0xC0) == 0x80);
					}
				}

			#pragma endregion

			#pragma region "Arrow Up"					("Up")

				static void arrow_up() {
					if (!history_length()) { beep(); return; }
					char *new_line = history_prev();

					if (!new_line) { beep(); return; }
					if (!tmp_line) tmp_line = strdup(std::string(buffer.value).substr(0, buffer.length).c_str());

					home(); delete_end(false);
					while (strlen(new_line) >= buffer.size) {
						buffer.value = (char *)realloc(buffer.value, buffer.size * 2);
						buffer.size *= 2;
					}
					strcpy(buffer.value, new_line);
					buffer.length = strlen(buffer.value);
					buffer.position = buffer.length;
					write_value(STDOUT_FILENO, buffer.value, buffer.length);
				}

			#pragma endregion

			#pragma region "Arrow Down"					("Down")

				static void arrow_down() {
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
					while (strlen(new_line) >= buffer.size) {
						buffer.value = (char *)realloc(buffer.value, buffer.size * 2);
						buffer.size *= 2;
					}
					strcpy(buffer.value, new_line);
					buffer.length = strlen(buffer.value);
					buffer.position = buffer.length;
					write_value(STDOUT_FILENO, buffer.value, buffer.length);

					if (free_line && new_line) free(new_line);
				}

			#pragma endregion

			#pragma region "Arrow Left"					("Left, CTRL + Left")

				// CURSOR MOVEMENT (CTRL + Left)
				//
				// Consists of a sequence of alphanumeric characters.
				// Any non-alphanumeric character is considered a delimiter or space.
				// An empty line is also considered to be a word.

				static void arrow_left(bool simple) {
					if (!buffer.ALT && !buffer.SHIFT && buffer.position > 0) {
						if (buffer.CTRL && !simple) {
							while (buffer.position > 0 && (std::isspace(buffer.value[buffer.position - 1]) || std::ispunct(buffer.value[buffer.position - 1]))) {
								cursor_left(0); (buffer.position)--;
							}
							while (buffer.position > 0 && !std::isspace(buffer.value[buffer.position - 1]) && !std::ispunct(buffer.value[buffer.position - 1])) {
								do { (buffer.position)--; } while (buffer.position > 0 && (buffer.value[buffer.position] & 0xC0) == 0x80);
								cursor_left(0);
							}
						} else if (buffer.position) {
							do { (buffer.position)--; } while (buffer.position > 0 && (buffer.value[buffer.position] & 0xC0) == 0x80);
							cursor_left(0);
						} else beep();
					} else beep();
				}

			#pragma endregion

			#pragma region "Arrow Right"				("Right, CTRL + Right")

				// CURSOR MOVEMENT (CTRL + Right)
				//
				// Consists of a sequence of alphanumeric characters.
				// Any non-alphanumeric character is considered a delimiter or space.
				// An empty line is also considered to be a word.

				static void arrow_right(bool simple) {
					if (!buffer.ALT && !buffer.SHIFT && buffer.position < buffer.length) {
						if (buffer.CTRL && !simple) {
							while (buffer.position < buffer.length && (std::isspace(buffer.value[buffer.position]) || std::ispunct(buffer.value[buffer.position]))) {
								cursor_right(0); (buffer.position)++;
							}
							while (buffer.position < buffer.length && !std::isspace(buffer.value[buffer.position]) && !std::ispunct(buffer.value[buffer.position])) {
								cursor_right(0);
								do { (buffer.position)++; } while (buffer.position < buffer.length && (buffer.value[buffer.position] & 0xC0) == 0x80);
							}
						} else {
							if (buffer.position < buffer.length) {
								cursor_right(0);
								do { (buffer.position)++; } while (buffer.position < buffer.length && (buffer.value[buffer.position] & 0xC0) == 0x80);
							} else beep();
						}
					} else beep();
				}

			#pragma endregion

		#pragma endregion

		#pragma region "Char"

			static int print_char() {
				size_t c_size = char_size(buffer.c);

				char *new_char = (char *)malloc(c_size + 1);
				new_char[0] = buffer.c;
				for (size_t i = 1; i < c_size; i++) read(STDIN_FILENO, &new_char[i], 1);
				new_char[c_size] = '\0';

				//	Ignore multi-space chars
				if (!multiwidth_chars && char_width(0, new_char) > 1) return (1);

				undo_push(true);

				// Expand buffer if necessary
				if (buffer.position + c_size >= buffer.size - 1) {
					buffer.value = (char *)realloc(buffer.value, buffer.size * 2);
					buffer.size *= 2;
				}

				if (buffer.position < buffer.length) memmove(&buffer.value[buffer.position + c_size], &buffer.value[buffer.position], buffer.length - buffer.position);

				// Insert all bytes of the character into the buffer
				for (size_t i = 0; i < c_size; i++) buffer.value[buffer.position++] = new_char[i];
				buffer.length += c_size;
				buffer.value[buffer.length] = '\0';

				write_value(STDOUT_FILENO, &buffer.value[buffer.position - c_size], buffer.length - (buffer.position - c_size));

				cursor_move(buffer.length, buffer.position);

				return (1);
			}

		#pragma endregion

	#pragma endregion

	#pragma region "Edit"

		#pragma region "Swap"							("CTRL + T, ALT + T")

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

							memcpy(temp, &buffer.value[buffer.position - back_pos1], back_pos1);
							memmove(&buffer.value[buffer.position - back_pos1], &buffer.value[buffer.position], back_pos2);
							buffer.position -= back_pos1; buffer.position += back_pos2;
							memmove(&buffer.value[buffer.position], temp, back_pos1);

							cursor_left(char_width(0, temp));
							write_value(STDOUT_FILENO, &buffer.value[buffer.position - back_pos2], back_pos1 + back_pos2);
							buffer.position += back_pos1;
						} else {
							size_t back_pos1 = 1, back_pos2 = 1;
							while (buffer.position - back_pos1 > 0 && (buffer.value[buffer.position - back_pos1] & 0xC0) == 0x80) back_pos1++;
							if (back_pos1 > 8) return;
							buffer.position -= back_pos1;
							while (buffer.position - back_pos2 > 0 && (buffer.value[buffer.position - back_pos2] & 0xC0) == 0x80) back_pos2++;

							memcpy(temp, &buffer.value[buffer.position - back_pos2], back_pos2);
							memmove(&buffer.value[buffer.position - back_pos2], &buffer.value[buffer.position], back_pos1);
							buffer.position -= back_pos2; buffer.position += back_pos1;
							memmove(&buffer.value[buffer.position], temp, back_pos2);

							cursor_left(char_width(0, temp));
							cursor_left(char_width(buffer.position - back_pos1, buffer.value));

							write_value(STDOUT_FILENO, &buffer.value[buffer.position - back_pos1], back_pos1 + back_pos2);
							buffer.position += back_pos2;
						}
					}
				}

			#pragma endregion

			#pragma region "Word"						("ALT + T")

				//	(Not working with multi-width chars 漢 🤬)
				static t_word get_word(size_t position, size_t len, bool prev_word) {
					t_word	word;

					if (prev_word) {
						if (position == len && position > 0) do { (position)--; } while (position > 0 && (buffer.value[position] & 0xC0) == 0x80);
						while (position > 0 && (std::isspace(buffer.value[position]) || std::ispunct(buffer.value[position]))) position--;
						if (position == 0) { word.len = 0; return (word); }
						while (position > 0 && !std::isspace(buffer.value[position]) && !std::ispunct(buffer.value[position])) position--;
						word.start = position + (std::isspace(buffer.value[position]) || std::ispunct(buffer.value[position]));
					} else {
						while (position < len && (std::isspace(buffer.value[position]) || std::ispunct(buffer.value[position]))) position++;
						if (position == len) { do { (position)--; } while (position > 0 && (buffer.value[position] & 0xC0) == 0x80);
							while (position > 0 && (std::isspace(buffer.value[position]) || std::ispunct(buffer.value[position]))) position--;
							while (position > 0 && !std::isspace(buffer.value[position]) && !std::ispunct(buffer.value[position])) position--;
						} else
							while (position > 0 && !std::isspace(buffer.value[position]) && !std::ispunct(buffer.value[position])) position--;

						word.start = position + (std::isspace(buffer.value[position]) || std::ispunct(buffer.value[position]));
					}

					size_t	temp_pos = word.start;
					while (temp_pos < len && !std::isspace(buffer.value[temp_pos]) && !std::ispunct(buffer.value[temp_pos]))
						do { temp_pos++; } while (temp_pos < len && (buffer.value[temp_pos] & 0xC0) == 0x80);
					word.end = temp_pos;

					word.len = word.end - word.start;

					if (word.len > 0 && word.len < sizeof(word.value))
						for (size_t i = 0, pos = word.start; i < word.len; i++, pos++)
							word.value[i] = buffer.value[pos];

					return (word);
				}

				static void swap_word() {
					t_word prev, next, sep;
					size_t temp_pos = buffer.position;

					if (buffer.position == buffer.length && buffer.position == 0) { beep(); return; }

					undo_push(false);

					next = get_word(buffer.position, buffer.length, false);
					if (next.len > sizeof(next.value)) { beep(); return; };
					if (!next.len) next = get_word(buffer.position, buffer.length, true);
					if (!next.len || next.start == 0) { beep(); return; };
					temp_pos = next.start - 1;
					prev = get_word(temp_pos, buffer.length, true);
					if (!prev.len || prev.len > sizeof(prev.value)) { beep(); return; };

					sep.start = prev.end;
					sep.end = next.start;
					if (sep.start < sep.end) {
						sep.len = sep.end - sep.start;
						if (sep.len > 0 && sep.len < sizeof(sep.value))
							for (size_t i = 0, pos = sep.start; i < sep.len; i++, pos++) sep.value[i] = buffer.value[pos];
					}

					memmove(&buffer.value[prev.start], next.value, next.len);
					memmove(&buffer.value[prev.start + next.len], sep.value, sep.len);
					memmove(&buffer.value[prev.start + next.len + sep.len], prev.value, prev.len);

					while (buffer.position > 0 && buffer.position > prev.start) {
						do { (buffer.position)--; } while (buffer.position > 0 && (buffer.value[buffer.position] & 0xC0) == 0x80);
						cursor_left(char_width(buffer.position, buffer.value));
					}

					write_value(STDOUT_FILENO, &buffer.value[buffer.position], prev.len + sep.len + next.len);
					buffer.position = prev.start + next.len + sep.len + prev.len;
				}

			#pragma endregion

		#pragma endregion

		#pragma region "Undo"							("CTRL + _")

			static void undo() {
				int len = chars_width(0, buffer.position, buffer.value);
				if (len > 0) cursor_left(len);

				len = chars_width(0, buffer.length, buffer.value);
				for (int i = len; i > 0; --i) write_value(STDOUT_FILENO, " ", 1);
				if (len > 0) cursor_left(len);

				undo_pop();

				write_value(STDOUT_FILENO, buffer.value, buffer.length);

				len = chars_width(buffer.position, buffer.length, buffer.value);
				if (len > 0) cursor_left(len);
			}

		#pragma endregion

		#pragma region "Clear Screen"					("CTRL + L")

			static void clear_screen() {
				write(STDOUT_FILENO, "\033[H\033[2J", 7);
				if (term_prompt) write(STDOUT_FILENO, term_prompt, strlen(term_prompt));
				write(STDOUT_FILENO, buffer.value, buffer.length);
				cursor_get();
				cursor_move(buffer.length, buffer.position);
			}

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
					memset(&seq, 0, sizeof(seq));
					if (buffer.c == 27) {

						fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
						int result = read(STDIN_FILENO, seq, sizeof(seq) - 1);
						fcntl(STDIN_FILENO, F_SETFL, O_SYNC);

						if (result > 0) {
							if (seq[0] == 't') { swap_word(); return (1); }						//	ALT + T			Swap word				(Not working with multi-width chars 漢 🤬)
							if (seq[0] == '[') { seq[1] = modifiers(seq);
								if (seq[1] == 'A') 						arrow_up();				//	Up				History prev
								if (seq[1] == 'B') 						arrow_down();			//	Down			History next
								if (seq[1] == 'D') 						arrow_left(0);			//	Left			Cursor left
								if (seq[1] == 'C') 						arrow_right(0);			//	Right			Cursor right
								if (seq[1] == 'H') 						home();					//	Home			Cursor to the start
								if (seq[1] == 'F')						end();					//	End				Cursor to the end
								if (seq[1] == '3' && seq[2] == '~')		delete_char();			//	Delete			Delete
								if (!strncmp(seq + 1, "3;5~", 4))	delete_word();			//	CTRL + Delete	Delete current word
							}
						} return (1);
					} return (0);
				}

			#pragma endregion

		#pragma endregion

		#pragma region "Specials"

			static int specials() {
				if (buffer.c == 127)						{ backspace();					}	//	[BackSpace]	Delete the previous character
				else if	(buffer.c == 1)						{ home();						}	//	[CTRL + A]	Cursor to the start
				else if (buffer.c == 2)						{ arrow_left(1);				}	//	[CTRL + B]	Cursor left
				else if (buffer.c == 4)						{ delete_char();				}	//	[CTRL + D]	Delete
				else if (buffer.c == 5)						{ end();						}	//	[CTRL + E]	Cursor to the end
				else if (buffer.c == 6)						{ arrow_right(1);				}	//	[CTRL + F]	Cursor right
				else if (buffer.c == 8)						{ backspace();					}	//	[CTRL + H]	Delete the previous character
				else if (buffer.c == 9)						{ autocomplete();				}	//	[Tab]		Auto-Complete
				else if (buffer.c == 10)					{ enter();						}	//	[CTRL + J]	Enter
				else if (buffer.c == 11)					{ delete_end(1);				}	//	[CTRL + K]	Delete from cursor to end
				else if (buffer.c == 12)					{ clear_screen();				}	//	[CTRL + L]	Clear screen
				else if (buffer.c == 14)					{ arrow_down();					}	//	[CTRL + N]	History prev
				else if (buffer.c == 16)					{ arrow_up();					}	//	[CTRL + P]	History next
				else if (buffer.c == 18)					{ history_search();				}	//	[CTRL + R]	History incremental search
				else if (buffer.c == 20)					{ swap_char();					}	//	[CTRL + T]	Swap the current character with the previous one
				else if (buffer.c == 21)					{ backspace_start();			}	//	[CTRL + U]	Backspace from cursor to the start of the line
				else if (buffer.c == 31)					{ undo();						}	//	[CTRL + _]	Undo last action
				else if (buffer.c >= 1 && buffer.c <= 26)	{ ;								}	//	Ignore other CTRL + X commands
				else if (buffer.c >= 28 && buffer.c <= 31)	{ ;								}	//	Ignore other CTRL + X commands
				else return (0);

				return (1);
			}

		#pragma endregion

	#pragma endregion

#pragma endregion

#pragma region "ReadLine"

	int readline(int readed) {
		int result = 0;

		if		(ctrl_d(readed))	result = 1;
		else if	(ctrl_c())			result = 1;
		else if	(enter())			result = 1;
		else if (specials())		result = 0;
		else if (cursor())			result = 0;
		else if (print_char())		result = 0;

		if (result && clipboard) free(clipboard);
		return (result);
	}

#pragma endregion
