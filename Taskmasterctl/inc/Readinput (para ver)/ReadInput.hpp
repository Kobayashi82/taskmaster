/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ReadInput.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 00:00:00 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/13 00:00:00 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <vector>
#include <memory>

namespace Terminal {

	enum class InputMode { 
		READLINE, 
		DUMB 
	};

	class Buffer {
	private:
		unsigned char	c;
		std::string		value;
		size_t			position;
		size_t			length;
		size_t			capacity;
		bool			shift_pressed;
		bool			alt_pressed;
		bool			ctrl_pressed;

	public:
		Buffer();
		~Buffer() = default;

		// Getters
		unsigned char	getChar() const { return c; }
		const std::string& getValue() const { return value; }
		size_t			getPosition() const { return position; }
		size_t			getLength() const { return length; }
		size_t			getCapacity() const { return capacity; }
		bool			isShiftPressed() const { return shift_pressed; }
		bool			isAltPressed() const { return alt_pressed; }
		bool			isCtrlPressed() const { return ctrl_pressed; }

		// Setters
		void			setChar(unsigned char ch) { c = ch; }
		void			setValue(const std::string& val);
		void			setPosition(size_t pos);
		void			setLength(size_t len) { length = len; }
		void			setCapacity(size_t cap);
		void			setShiftPressed(bool pressed) { shift_pressed = pressed; }
		void			setAltPressed(bool pressed) { alt_pressed = pressed; }
		void			setCtrlPressed(bool pressed) { ctrl_pressed = pressed; }

		// Buffer operations
		void			clear();
		void			resize(size_t new_size);
		void			insertChar(char ch, size_t pos);
		void			deleteChar(size_t pos);
		void			insertString(const std::string& str, size_t pos);
		void			deleteRange(size_t start, size_t end);
	};

	class ReadInput {
	private:
		static Buffer			buffer;
		static std::string		term_prompt;
		static bool				raw_mode;
		static bool				hist_searching;
		static InputMode		input_mode;

		// Private methods
		static void		enableRawMode();
		static void		disableRawMode();
		static int		checkTTY();
		static void		restoreTerminal();

	public:
		ReadInput() = delete;  // Static class
		~ReadInput() = delete;

		// Main interface
		static std::string	readInput(const std::string& prompt);
		static std::string	getInput();

		// Mode handlers
		static int			readline(int readed);
		static int			dumb(int readed);

		// Features
		static void			autocomplete();
		static int			historySearch();

		// Undo/Redo system
		static void			undoPush(bool push);
		static void			undoPop();
		static void			undoAll();
		static void			undoClear();

		// Getters
		static const Buffer& getBuffer() { return buffer; }
		static const std::string& getPrompt() { return term_prompt; }
		static bool			isRawMode() { return raw_mode; }
		static bool			isHistSearching() { return hist_searching; }
		static InputMode	getInputMode() { return input_mode; }

		// Setters
		static void			setInputMode(InputMode mode) { input_mode = mode; }
		static void			setHistSearching(bool searching) { hist_searching = searching; }

		// Cleanup
		static void			cleanup();
	};

}