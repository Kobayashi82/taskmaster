/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AutoComplete.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 00:00:00 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/13 22:08:00 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Readinput/ReadInput.hpp"
#include "Readinput/Termcaps.hpp"
#include <vector>
#include <string>
#include <algorithm>
#include <unistd.h>
#include <dirent.h>

namespace Terminal {

    // Helper function to get completions
    static std::vector<std::string> getCompletions(const std::string& prefix) {
        std::vector<std::string> completions;
        
        // Simple file completion for now
        // In a real implementation, this would be more sophisticated
        DIR* dir = opendir(".");
        if (dir) {
            struct dirent* entry;
            while ((entry = readdir(dir)) != nullptr) {
                std::string name = entry->d_name;
                if (name.length() >= prefix.length() && 
                    name.substr(0, prefix.length()) == prefix) {
                    completions.push_back(name);
                }
            }
            closedir(dir);
        }
        
        std::sort(completions.begin(), completions.end());
        return completions;
    }

    // Helper function to find common prefix
    static std::string findCommonPrefix(const std::vector<std::string>& completions) {
        if (completions.empty()) {
            return "";
        }
        
        if (completions.size() == 1) {
            return completions[0];
        }
        
        std::string common = completions[0];
        for (size_t i = 1; i < completions.size(); ++i) {
            while (!completions[i].empty() && 
                   (common.length() > completions[i].length() || 
                    completions[i].substr(0, common.length()) != common)) {
                common.pop_back();
            }
        }
        
        return common;
    }

    void autoCompleteFunction() {
        auto& buffer = const_cast<Buffer&>(ReadInput::getBuffer());
        
        // Find the word at cursor position
        std::string line = buffer.getValue();
        size_t pos = buffer.getPosition();
        
        // Find word boundaries
        size_t start = pos;
        while (start > 0 && line[start - 1] != ' ' && line[start - 1] != '\t') {
            start--;
        }
        
        size_t end = pos;
        while (end < line.length() && line[end] != ' ' && line[end] != '\t') {
            end++;
        }
        
        std::string word = line.substr(start, end - start);
        
        // Get completions
        std::vector<std::string> completions = getCompletions(word);
        
        if (completions.empty()) {
            // No completions found - maybe bell or visual feedback
            return;
        }
        
        if (completions.size() == 1) {
            // Single completion - complete it
            std::string completion = completions[0];
            std::string to_insert = completion.substr(word.length());
            
            if (!to_insert.empty()) {
                buffer.insertString(to_insert, pos);
                
                // Redraw the line
                std::string remaining = buffer.getValue().substr(pos);
                write(STDOUT_FILENO, remaining.c_str(), remaining.length());
                
                // Move cursor to the end of the completion
                size_t new_pos = pos + to_insert.length();
                if (new_pos < buffer.getLength()) {
                    for (size_t i = new_pos; i < buffer.getLength(); ++i) {
                        Termcaps::cursorLeft(1);
                    }
                }
                
                const_cast<Buffer&>(buffer).setPosition(new_pos);
            }
        } else {
            // Multiple completions - show common prefix or list options
            std::string common = findCommonPrefix(completions);
            
            if (common.length() > word.length()) {
                // Complete up to common prefix
                std::string to_insert = common.substr(word.length());
                buffer.insertString(to_insert, pos);
                
                // Redraw the line
                std::string remaining = buffer.getValue().substr(pos);
                write(STDOUT_FILENO, remaining.c_str(), remaining.length());
                
                // Move cursor to the end of the completion
                size_t new_pos = pos + to_insert.length();
                if (new_pos < buffer.getLength()) {
                    for (size_t i = new_pos; i < buffer.getLength(); ++i) {
                        Termcaps::cursorLeft(1);
                    }
                }
                
                const_cast<Buffer&>(buffer).setPosition(new_pos);
            } else {
                // Show all completions
                write(STDOUT_FILENO, "\n", 1);
                
                for (const auto& completion : completions) {
                    write(STDOUT_FILENO, completion.c_str(), completion.length());
                    write(STDOUT_FILENO, "  ", 2);
                }
                
                write(STDOUT_FILENO, "\n", 1);
                
                // Redraw prompt and current line
                if (!ReadInput::getPrompt().empty()) {
                    write(STDOUT_FILENO, ReadInput::getPrompt().c_str(), ReadInput::getPrompt().length());
                }
                
                if (!buffer.getValue().empty()) {
                    write(STDOUT_FILENO, buffer.getValue().c_str(), buffer.getValue().length());
                    
                    // Move cursor to correct position
                    for (size_t i = buffer.getPosition(); i < buffer.getLength(); ++i) {
                        Termcaps::cursorLeft(1);
                    }
                }
            }
        }
    }

}

// Update the ReadInput::autocomplete method to use this implementation
namespace Terminal {
    void ReadInput::autocomplete() {
        autoCompleteFunction();
    }
}