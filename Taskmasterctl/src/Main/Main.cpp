/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/11 19:29:12 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/13 22:13:14 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Readinput/ReadInput.hpp"
	#include "Readinput/History.hpp"
	#include <iostream>
	#include <string>

#pragma endregion

#pragma region "Main"

	int main(int argc, char **argv) {
		(void) argc;
		(void) argv;

		int result = 0;

		// Initialize the history system
		Terminal::History::initialize();
		
		// Set input mode (can be changed via options later)
		Terminal::ReadInput::setInputMode(Terminal::InputMode::READLINE);

		std::cout << "Taskmaster Control Terminal (C++ Readline)" << std::endl;
		std::cout << "Type 'exit' to quit, 'history' to show command history" << std::endl;
		std::cout << "Use Ctrl+R for history search, Tab for completion" << std::endl;

		std::string input;
		while (true) {
			try {
				// Read input using the new C++ readline
				input = Terminal::ReadInput::readInput("taskmaster> ");
				
				if (input.empty()) {
					continue;
				}
				
				// Simple command processing
				if (input == "exit" || input == "quit") {
					break;
				} else if (input == "history") {
					std::cout << "Command History:" << std::endl;
					Terminal::History::print();
				} else if (input == "clear") {
					std::cout << "\033[2J\033[H"; // Clear screen
				} else {
					std::cout << "Unknown command: " << input << std::endl;
					std::cout << "Available commands: exit, quit, history, clear" << std::endl;
				}
			} catch (const std::exception& e) {
				std::cerr << "Error: " << e.what() << std::endl;
				result = 1;
				break;
			}
		}

		// Cleanup
		Terminal::ReadInput::cleanup();
		Terminal::History::cleanup();

		return (result);
	}

#pragma endregion

