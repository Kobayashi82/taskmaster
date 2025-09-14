/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   History.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/05 09:43:32 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/14 15:11:10 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Readline/History.hpp"

	#include <iostream>
	#include <iomanip>

#pragma endregion

#pragma region "Constructors"

	History::History() : position(0), begining(false), middle(false) {}

#pragma endregion

#pragma region "Add"

	//	Add an entry to the history
	int History::add(const std::string& line) {
		if (line.empty())								return (1);
		if (!history.empty() && history.back() == line)	return (1);
		if (std::isspace(line.front()))					return (1);

		history.push_back(line);
		position = history.size() - 1;
		begining = middle = false;

		return (0);
	}

#pragma endregion

#pragma region "Clear"

	//	Clear all entries
	void History::clear() {
		history.clear();
		begining = middle = position = 0;
	}

	#pragma endregion

#pragma region "Print"

	//	Print all entries
	void History::print(size_t offset, bool hide_events) const {
		if (history.empty()) return;

		size_t len = history.size();
		if (offset > len) offset = len;

		for (size_t i = len - offset; i < len; ++i) {
			if (!hide_events) std::cout << std::setw(5) << i << "  ";
			std::cout << history[i] << "\n";
		}
	}

#pragma endregion

#pragma region "Get"

	#pragma region "Length"

		//	Return the length of the history
		size_t History::length() const {
			return (history.size());
		}

	#pragma endregion

	#pragma region "Entry"

		//	Return a pointer to the indicated entry
		std::string History::get(size_t pos) const {
			return ((pos < history.size()) ? history[pos] : "");
		}

		//	Return a pointer to the current entry
		std::string History::get_current() const {
			return ((position < history.size()) ? history[position] : "");
		}

	#pragma endregion

#pragma endregion

#pragma region "Navigate"

	#pragma region "Previous"

		//	Return the previous entry line
		std::string History::prev() {
			if (history.empty()) return {};

			if (position == history.size()) position = history.size() -1;
			if (begining) return {};
			if (position > 0 && middle) --position;
			if (position == 0) begining = true;
			middle = true;

			return (history[position]);
		}

	#pragma endregion

	#pragma region "Next"

		//	Return the next entry line
		std::string History::next() {
			if (history.empty()) return {};

			if (position == history.size()) position = history.size() -1;

			begining = false; middle = true;

			if (position >= history.size() - 1) { middle = false; return {}; }
			++position;

			return (history[position]);
		}

	#pragma endregion

	#pragma region "Get Position"

		//	Return the current position in the history
		size_t History::get_pos() const {
			return (position);
		}

	#pragma endregion

	#pragma region "Set Position"

		//	Change the position in the history
		void History::set_pos(size_t pos) {
			if		(history.empty())			position = 0;
			else if	(pos >= history.size())		position = history.size() - 1;
			else								position = pos;
		}

		//	Set the position in the history to the last entry
		void History::set_pos_end() {
			begining = middle = false;
			position = (history.empty()) ? 0 : history.size() - 1;
		}

	#pragma endregion

#pragma endregion
