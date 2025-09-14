/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HistorySearch.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 00:00:00 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/14 12:54:37 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Readline/ReadInput.hpp"
#include "Readline/History.hpp"
#include "Readline/Termcaps.hpp"
#include <unistd.h>

namespace Terminal {

    // Static variables for history search
    static std::string search_pattern;
    static size_t search_start_position = 0;
    static bool search_direction_backward = true;

    // Helper function to search history
    static HistoryEntry* searchInHistory(const std::string& pattern, bool backward) {
        size_t history_length = History::length();
        if (history_length == 0 || pattern.empty()) {
            return nullptr;
        }

        size_t current_pos = History::getPosition();
        
        if (backward) {
            // Search backward through history
            for (size_t i = 0; i < history_length; ++i) {
                size_t check_pos = (current_pos + history_length - 1 - i) % history_length;
                HistoryEntry* entry = History::get(check_pos);
                
                if (entry && entry->getLine().find(pattern) != std::string::npos) {
                    History::setPosition(check_pos);
                    return entry;
                }
            }
        } else {
            // Search forward through history
            for (size_t i = 1; i <= history_length; ++i) {
                size_t check_pos = (current_pos + i) % history_length;
                HistoryEntry* entry = History::get(check_pos);
                
                if (entry && entry->getLine().find(pattern) != std::string::npos) {
                    History::setPosition(check_pos);
                    return entry;
                }
            }
        }
        
        return nullptr;
    }

    // Helper function to update display with search result
    static void updateSearchDisplay(const std::string& pattern, HistoryEntry* entry) {
        auto& buffer = const_cast<Buffer&>(ReadInput::getBuffer());
        
        // Clear current line
        Termcaps::cursorMove(buffer.getPosition(), 0);
        std::string spaces(buffer.getLength() + 20, ' '); // Extra spaces to clear
        write(STDOUT_FILENO, spaces.c_str(), spaces.length());
        Termcaps::cursorMove(spaces.length(), 0);
        
        // Show search prompt
        std::string search_prompt = "(search): " + pattern;
        write(STDOUT_FILENO, search_prompt.c_str(), search_prompt.length());
        
        if (entry) {
            // Show found entry
            write(STDOUT_FILENO, " -> ", 4);
            write(STDOUT_FILENO, entry->getLine().c_str(), entry->getLine().length());
            
            // Update buffer with found entry
            const_cast<Buffer&>(buffer).setValue(entry->getLine());
            const_cast<Buffer&>(buffer).setPosition(entry->getLine().length());
        } else {
            // No match found
            write(STDOUT_FILENO, " (no match)", 11);
        }
    }

    int historySearchFunction() {
        auto& buffer = const_cast<Buffer&>(ReadInput::getBuffer());
        unsigned char c = buffer.getChar();
        
        switch (c) {
            case 27: { // Escape - exit search
                ReadInput::setHistSearching(false);
                
                // Restore normal display
                Termcaps::cursorMove(buffer.getPosition(), 0);
                std::string spaces(buffer.getLength() + 50, ' ');
                write(STDOUT_FILENO, spaces.c_str(), spaces.length());
                Termcaps::cursorMove(spaces.length(), 0);
                
                // Show prompt and current buffer
                if (!ReadInput::getPrompt().empty()) {
                    write(STDOUT_FILENO, ReadInput::getPrompt().c_str(), ReadInput::getPrompt().length());
                }
                
                if (!buffer.getValue().empty()) {
                    write(STDOUT_FILENO, buffer.getValue().c_str(), buffer.getValue().length());
                }
                
                return 1;
            }
                
            case 10: // Enter - accept current selection
            case 13: {
                ReadInput::setHistSearching(false);
                
                // Clear search display and show normal prompt
                Termcaps::cursorMove(buffer.getPosition(), 0);
                std::string spaces(buffer.getLength() + 50, ' ');
                write(STDOUT_FILENO, spaces.c_str(), spaces.length());
                Termcaps::cursorMove(spaces.length(), 0);
                
                if (!ReadInput::getPrompt().empty()) {
                    write(STDOUT_FILENO, ReadInput::getPrompt().c_str(), ReadInput::getPrompt().length());
                }
                
                if (!buffer.getValue().empty()) {
                    write(STDOUT_FILENO, buffer.getValue().c_str(), buffer.getValue().length());
                }
                
                return 1;
            }
                
            case 127: { // Backspace - remove last character from search
                if (!search_pattern.empty()) {
                    search_pattern.pop_back();
                    
                    // Search again with updated pattern
                    HistoryEntry* entry = searchInHistory(search_pattern, search_direction_backward);
                    updateSearchDisplay(search_pattern, entry);
                }
                return 1;
            }
                
            case 18: { // Ctrl+R - search backward (repeat search)
                search_direction_backward = true;
                HistoryEntry* entry = searchInHistory(search_pattern, true);
                updateSearchDisplay(search_pattern, entry);
                return 1;
            }
                
            case 19: { // Ctrl+S - search forward
                search_direction_backward = false;
                HistoryEntry* entry = searchInHistory(search_pattern, false);
                updateSearchDisplay(search_pattern, entry);
                return 1;
            }
                
            default: {
                // Add character to search pattern
                if (c >= 32 && c <= 126) { // Printable ASCII
                    search_pattern += static_cast<char>(c);
                    
                    // Search with updated pattern
                    HistoryEntry* entry = searchInHistory(search_pattern, search_direction_backward);
                    updateSearchDisplay(search_pattern, entry);
                }
                return 1;
            }
        }
    }

    // Function to start history search
    void startHistorySearch() {
        ReadInput::setHistSearching(true);
        search_pattern.clear();
        search_start_position = History::getPosition();
        search_direction_backward = true;
        
        // Show initial search prompt
        auto& buffer = const_cast<Buffer&>(ReadInput::getBuffer());
        Termcaps::cursorMove(buffer.getPosition(), 0);
        std::string spaces(buffer.getLength() + 20, ' ');
        write(STDOUT_FILENO, spaces.c_str(), spaces.length());
        Termcaps::cursorMove(spaces.length(), 0);
        
        write(STDOUT_FILENO, "(search): ", 10);
    }

}

// Update the ReadInput::historySearch method to use this implementation
namespace Terminal {
    int ReadInput::historySearch() {
        return historySearchFunction();
    }
}