/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   history.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/05 09:43:32 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/13 23:59:24 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// SIGHUP, SIGTERM para guardar historial

#pragma region "Includes"

	#include "readinput/history.hpp"

	#include <string>
	#include <cstring>
	#include <algorithm>
	#include <stdlib.h>
	#include <climits>
	#include <fcntl.h>
	#include <unistd.h>

#pragma endregion

#pragma region "Variables"

	#define HIST_MAXSIZE 5000

	//.42shrc
	static HIST_ENTRY	**history			= NULL;		//	History array

	static char			history_file[4096];				//	Path to the physical history file
	static size_t		file_max			= 2000;		//	Maximum number of entry
	static bool			file_unlimited		= false;	//	Indicates if it is limited by a maximum size

	static size_t		mem_max				= 1000;		//	Maximum number of entry
	static bool			mem_unlimited		= false;	//	Indicates if it is limited by a maximum size

	static size_t		length				= 0;		//	Current number of entry
	static size_t		capacity			= 10;		//	Array size
	static size_t		event				= 1;		//	Current event number

	static size_t		position			= 0;		//	Current position within the array

	static bool			begining			= false;
	static bool			middle				= false;
	static bool			added				= false;

#pragma endregion

#pragma region "File"

	void history_file_set(const char *filename) {
		if (!filename) return;
		strcpy(history_file, filename);
	}

#pragma endregion

#pragma region "Size"

	#pragma region "Get Size"

		//	Get the maximum size for the history
		size_t history_get_size(int type) {
			if (type == HIST_MEM)	{ return (mem_max); }
			if (type == HIST_FILE)	{ return (file_max); }
			return (mem_max);
		}

	#pragma endregion

	#pragma region "Set Size"

		//	Set a maximum size for the history
		void history_set_size(size_t value, int type) {
			size_t new_size = std::min(std::max((size_t)0, value), (size_t)HIST_MAXSIZE);

			if (type == HIST_FILE) { file_max = new_size; file_unlimited = false; }
			if (type == HIST_MEM)  { mem_max = new_size;  mem_unlimited = false;
				if (mem_max < length) {
					HIST_ENTRY **tmp_history = (HIST_ENTRY **)calloc((mem_max * 2) + 1, sizeof(HIST_ENTRY *));
					size_t i = 0;

					for (size_t start = length - mem_max; start < length && history[start]; ++start) {
						tmp_history[i++] = history[start]; history[start] = NULL;
					} tmp_history[i] = NULL;
					history_clear();

					length = mem_max;
					capacity = std::max((size_t)0, mem_max * 2);
					position = length > 0 ? length - 1 : 0;
					history = tmp_history;
				}
			}
		}

	#pragma endregion

	#pragma region "Unset Size"

		//	Remove the size limitation for the history
		void history_unset_size(int type) {
			if (type == HIST_MEM)	{  mem_max = INT_MAX;  mem_unlimited = true; }
			if (type == HIST_FILE)	{ file_max = INT_MAX; file_unlimited = true; }
		}

	#pragma endregion

	#pragma region "Resize"

		//	Initialize or resize the history
		//	- If `initialize` is true or `history` is NULL, it sets up a new buffer with a default capacity
		//	- If the buffer is full, it doubles the capacity by a power of 2 and preserves existing entries
		static void history_resize(bool initialize) {
			if (initialize || !history) {
				if (history) history_clear();
				capacity = 10; length = 0;
				history = (HIST_ENTRY **)calloc(capacity + 1, sizeof(HIST_ENTRY *));
			} else if (length == capacity) {
				capacity *= 2;
				HIST_ENTRY **new_history = (HIST_ENTRY **)calloc(capacity + 1, sizeof(HIST_ENTRY *));
				for (size_t i = 0; i < length && history[i]; ++i)
					new_history[i] = history[i];
				free(history);
				history = new_history;
			}
		}

	#pragma endregion

#pragma endregion

#pragma region "Add"

		//	Remove copies of the same line in the history
		static void erase_dups(const char *line, size_t pos) {
			if (!history || !length) return;

			size_t i = length;
			while (i-- > 0 && history[i])
				if (i != pos && !strcmp(history[i]->line, line)) history_remove(i);
		}

	#pragma region "Add"

		//	Add an entry to the history
		int history_add(char *line, bool force) {
			if (!line || std::string(line).empty() || !mem_max) return (1);

			bool ignoredups = true, ignorespace = true, erasedups = false;

			if (!force && ignoredups && length && history && history[length - 1] && history[length - 1]->line && !strcmp(history[length - 1]->line, line)) { added = false; return (1); }
			if (!force && ignorespace && std::isspace(*line)) { added = false; return (1); }
			if (!force && erasedups) erase_dups(line, INT_MAX);

			history_resize(false);
			if (length >= mem_max && !mem_unlimited) {
				free(history[0]->line);
				free(history[0]->data);
				free(history[0]); history[0] = NULL;
				for (size_t i = 0; i < length; ++i)
					history[i] = history[i + 1];
				length -= 1;
			}
			history[length] = (HIST_ENTRY *)malloc(sizeof(HIST_ENTRY));
			history[length]->line = strdup(line);
			history[length]->length = strlen(line);
			history[length]->event = event++;
			history[length++]->data = NULL;
			history[length] = NULL;
			history_set_pos_end();
			added = true;

			begining = middle = false;

			return (0);
		}

	#pragma endregion

	#pragma region "Replace"

		//	Replace the indicated entry
		int history_replace(size_t pos, char *line, void *data) {
			if (!history || !line || std::string(line).empty() || !length || !mem_max) return (1);

			bool ignoredups = true, ignorespace = true, erasedups = false;

			if (pos > 0 && ignoredups && history[pos - 1] && history[pos - 1]->line && !strcmp(history[pos - 1]->line, line)) return (1);
			if (pos == length - 1 && ignoredups && history[pos + 1] && history[pos + 1]->line && !strcmp(history[pos + 1]->line, line)) return (1);
			if (ignorespace && std::isspace(*line)) return (1);
			if (erasedups) erase_dups(line, pos);

			if (history && pos < length && history[pos]) {
				if (history[pos]->line) free(history[pos]->line);
				if (history[pos]->data) free(history[pos]->data);
				history[pos]->line = strdup(line);
				history[pos]->length = strlen(line);
				history[pos]->data = data;
			}

			return (0);
		}

	#pragma endregion

#pragma endregion

#pragma region "Delete"

	#pragma region "Remove"

		//	Remove the indicate entry by an offset
		void history_remove_offset(int offset) {
			if (!history || length == 0) return;

			size_t pos;
			if (offset < 0) {
				if ((size_t)(-offset) > length) return;				//	Negative offset out of range
				pos = length + offset;
			} else {
				if (!offset || (size_t)offset > length) return;		//	Positivo offset out of range
				pos = offset - 1;
			}

			if (history && pos < length && history[pos]) {
				if (history[pos]->line) free(history[pos]->line);
				if (history[pos]->data) free(history[pos]->data);
				free(history[pos]); history[pos] = NULL;
				for (size_t i = pos; i < length; ++i)
					history[i] = history[i + 1];
				length -= 1;
				if (length && position == length) { history_set_pos_end(); added = false; }
			}
		}

		//	Remove the indicate entry
		void history_remove(size_t pos) {
			if (!history || length == 0) return;

			if (history && pos < length && history[pos]) {
				if (history[pos]->line) free(history[pos]->line);
				if (history[pos]->data) free(history[pos]->data);
				free(history[pos]); history[pos] = NULL;
				for (size_t i = pos; i < length; ++i)
					history[i] = history[i + 1];
				length -= 1;
				if (length && position == length) { history_set_pos_end(); added = false; }
			}
		}

		//	Remove the indicate event
		void history_remove_event(size_t event) {
			if (!history || length == 0) return;

			for (size_t i = 0; i < length && history[i]; ++i)
				if (history[i]->event == event) history_remove(i);
		}

		//	Remove the current entry
		void history_remove_current(bool remove_event) {
			if (!history || length == 0) return;

			if (history && position < length && history[position]) {
				if (history[position]->line) free(history[position]->line);
				if (history[position]->data) free(history[position]->data);
				free(history[position]); history[position] = NULL;
				for (size_t i = position; i < length; ++i)
					history[i] = history[i + 1];
				length -= 1;
				if (length && position >= length) { history_set_pos_end(); added = false; }
				if (remove_event) event--;
			}
		}

		//	Remove the last entry
		void history_remove_last_if_added(bool remove_event) {
			if (!history || length == 0 || !added) return;

			history_set_pos_end();
			history_remove_current(remove_event);
			added = false;
		}

	#pragma endregion

	#pragma region "Clear"

		//	Clear all entries
		void history_clear() {
			if (!history) return;

			for (size_t i = 0; i < length; ++i) {
				if (history && history[i]) {
					if (history[i]->line) free(history[i]->line);
					if (history[i]->data) free(history[i]->data);
					free(history[i]); history[i] = NULL;
				}
			}
			if (history) { free(history); history = NULL; }
			position = 0; length = 0; capacity = 10;
		}

	#pragma endregion

#pragma endregion

#pragma region "Get"

	#pragma region "Clone"

		//	Return a clone of the history
		HIST_ENTRY **history_clone() {
			if (!history) return (NULL);

			HIST_ENTRY **copy = (HIST_ENTRY **)malloc((length + 1) * sizeof(HIST_ENTRY *));

			for (size_t i = 0; i < length && history[i]; ++i)
				copy[i] = history[i];
			copy[length] = NULL;

			return (copy);
		}

	#pragma endregion

	#pragma region "Length"

		//	Return the length of the history
		size_t history_length() {
			return (length);
		}

	#pragma endregion

	#pragma region "Get"

		//	Return a pointer to the indicated entry
		HIST_ENTRY *history_get(size_t pos) {
			if (history && pos < length) return (history[pos]);
			return (NULL);
		}

		//	Return a pointer to the current entry
		HIST_ENTRY *history_current() {
			if (history && position < length) return (history[position]);
			return (NULL);
		}

		//	Return a pointer to the last entry if added
		HIST_ENTRY *history_get_last_if_added() {
			if (!history || length == 0 || !added) return (NULL);

			history_set_pos_end();
			return (history_current());
		}

	#pragma endregion

	#pragma region "Event"

		//	Return a pointer to the entry with the indicated event
		HIST_ENTRY *history_event(size_t event) {
			if (!history) return (NULL);

			for (size_t i = 0; i < length && history[i]; ++i)
				if (history[i]->event == event) return (history[i]);

			return (NULL);
		}

		//	Return the position to the entry with the indicated event
		int history_event_pos(size_t event) {
			if (!history) return (-1);

			for (size_t i = 0; i < length && history[i]; ++i)
				if (history[i]->event == event) return (i);

			return (-1);
		}

	#pragma endregion

#pragma endregion

#pragma region "Navigate"

	#pragma region "Previous"

		//	Return the previous entry line
		char *history_prev() {
			if (!history || !mem_max || !length) return (NULL);

			if (position == length) position = length -1;
			if (begining) return (NULL);
			if (position > 0 && middle) position--;
			if (position == 0) begining = true;
			middle = true;

			return (history[position]->line);
		}

	#pragma endregion

	#pragma region "Next"

		//	Return the next entry line
		char *history_next() {
			if (!history || !mem_max || !length) return (NULL);

			if (position == length) position = length -1;
			begining = false; middle = true;
			if (position >= length - 1) { middle = false; return (NULL); }
			position++;

			return (history[position]->line);
		}

	#pragma endregion

	#pragma region "Get Position"

		//	Return the current position in the history
		size_t history_get_pos() {
			return (position);
		}

	#pragma endregion

	#pragma region "Set Position"

		//	Change the position in the history
		void history_set_pos(size_t pos) {
			position = std::min(std::max(pos, (size_t)0), length - 1);
		}

		//	Set the position in the history to the last entry
		void history_set_pos_end() {
			begining = false; middle = false;
			position = 0;
			if (length > 0) position = length - 1;
		}

	#pragma endregion

#pragma endregion

#pragma region "Print"

	//	Print all entries
	// int history_print(size_t offset, bool hide_events) {
	// 	if (!history || !length) return (1);
	// 	if (offset > length) offset = length;

	// 	print(STDOUT_FILENO, NULL, RESET);

	// 	for (size_t i = length - offset; i < length && history[i]; ++i) {
	// 		if (!hide_events) {
	// 			char *txt_event = std::to_string(history[i]->event);
	// 			int spaces = 5 - strlen(txt_event);
	// 			while (spaces--) print(STDOUT_FILENO, " ", JOIN);
	// 			print(STDOUT_FILENO, txt_event, FREE_JOIN);
	// 			print(STDOUT_FILENO, "  ", JOIN);
	// 		}
	// 		print(STDOUT_FILENO, history[i]->line, JOIN);
	// 		print(STDOUT_FILENO, "\n", JOIN);
	// 	}

	// 	print(STDOUT_FILENO, NULL, PRINT);

	// 	return (0);
	// }

#pragma endregion

#pragma region "Initialize"

	//	Initialize the history
	int history_initialize() {
		history_resize(true);

		return (0);
	}

#pragma endregion
