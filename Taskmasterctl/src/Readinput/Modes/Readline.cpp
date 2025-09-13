/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Readline.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 00:00:00 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/13 00:00:00 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Readinput/ReadInput.hpp"
#include "Readinput/Termcaps.hpp"
#include "Readinput/History.hpp"
#include <unistd.h>

namespace Terminal {

    // Simple, working readline implementation
    int readlineMode(int readed) {
        auto& buffer = const_cast<Buffer&>(ReadInput::getBuffer());
        
        if (readed <= 0) {
            return -1;  // EOF
        }

        unsigned char c = buffer.getChar();

        switch (c) {
            case 1:   // Ctrl+A (Home)
                {
                    // Move to beginning of line
                    size_t pos = buffer.getPosition();
                    for (size_t i = 0; i < pos; ++i) {
                        write(STDOUT_FILENO, "\b", 1);
                    }
                    const_cast<Buffer&>(buffer).setPosition(0);
                }
                return 0;
                
            case 4:   // Ctrl+D (EOF)
                if (buffer.getLength() == 0) {
                    return -1;  // Exit on empty line
                }
                // Delete character at cursor (like delete key)
                if (buffer.getPosition() < buffer.getLength()) {
                    std::string value = buffer.getValue();
                    value.erase(buffer.getPosition(), 1);
                    buffer.setValue(value);
                    
                    // Redraw from cursor to end
                    std::string remaining = value.substr(buffer.getPosition());
                    remaining += " ";  // Clear last char
                    write(STDOUT_FILENO, remaining.c_str(), remaining.length());
                    
                    // Move cursor back
                    for (size_t i = 0; i < remaining.length() - 1; ++i) {
                        write(STDOUT_FILENO, "\b", 1);
                    }
                }
                return 0;
                
            case 5:   // Ctrl+E (End)
                {
                    // Move to end of line
                    size_t pos = buffer.getPosition();
                    size_t len = buffer.getLength();
                    for (size_t i = pos; i < len; ++i) {
                        write(STDOUT_FILENO, "\033[C", 3);
                    }
                    const_cast<Buffer&>(buffer).setPosition(len);
                }
                return 0;
                
            case 8:   // Ctrl+H (Backspace)
            case 127: // DEL (Backspace)
                if (buffer.getPosition() > 0) {
                    // Move cursor back
                    write(STDOUT_FILENO, "\b", 1);
                    const_cast<Buffer&>(buffer).setPosition(buffer.getPosition() - 1);
                    
                    // Delete character from buffer
                    std::string value = buffer.getValue();
                    value.erase(buffer.getPosition(), 1);
                    buffer.setValue(value);
                    
                    // Redraw from cursor to end
                    std::string remaining = value.substr(buffer.getPosition());
                    remaining += " ";  // Clear last char
                    write(STDOUT_FILENO, remaining.c_str(), remaining.length());
                    
                    // Move cursor back to correct position
                    for (size_t i = 0; i < remaining.length() - 1; ++i) {
                        write(STDOUT_FILENO, "\b", 1);
                    }
                }
                return 0;
                
            case 9:   // Tab (Autocomplete)
                ReadInput::autocomplete();
                return 0;
                
            case 10:  // Enter/Return
            case 13:
                write(STDOUT_FILENO, "\n", 1);
                return 1;  // Line complete
                
            case 27:  // ESC sequence (arrow keys, etc.)
                {
                    unsigned char seq[3];
                    if (read(STDIN_FILENO, &seq[0], 1) == 1) {
                        if (seq[0] == '[') {
                            if (read(STDIN_FILENO, &seq[1], 1) == 1) {
                                switch (seq[1]) {
                                    case 'A': // Up arrow - previous history
                                        {
                                            std::string prev = History::prev();
                                            if (!prev.empty()) {
                                                // Clear current line
                                                size_t pos = buffer.getPosition();
                                                for (size_t i = 0; i < pos; ++i) {
                                                    write(STDOUT_FILENO, "\b", 1);
                                                }
                                                std::string spaces(buffer.getLength(), ' ');
                                                write(STDOUT_FILENO, spaces.c_str(), spaces.length());
                                                for (size_t i = 0; i < buffer.getLength(); ++i) {
                                                    write(STDOUT_FILENO, "\b", 1);
                                                }
                                                
                                                // Set new content
                                                buffer.setValue(prev);
                                                write(STDOUT_FILENO, prev.c_str(), prev.length());
                                                buffer.setPosition(prev.length());
                                            }
                                        }
                                        break;
                                        
                                    case 'B': // Down arrow - next history
                                        {
                                            std::string next = History::next();
                                            // Clear current line
                                            size_t pos = buffer.getPosition();
                                            for (size_t i = 0; i < pos; ++i) {
                                                write(STDOUT_FILENO, "\b", 1);
                                            }
                                            std::string spaces(buffer.getLength(), ' ');
                                            write(STDOUT_FILENO, spaces.c_str(), spaces.length());
                                            for (size_t i = 0; i < buffer.getLength(); ++i) {
                                                write(STDOUT_FILENO, "\b", 1);
                                            }
                                            
                                            // Set new content
                                            buffer.setValue(next);
                                            if (!next.empty()) {
                                                write(STDOUT_FILENO, next.c_str(), next.length());
                                                buffer.setPosition(next.length());
                                            } else {
                                                buffer.setPosition(0);
                                            }
                                        }
                                        break;
                                        
                                    case 'C': // Right arrow
                                        if (buffer.getPosition() < buffer.getLength()) {
                                            write(STDOUT_FILENO, "\033[C", 3);
                                            const_cast<Buffer&>(buffer).setPosition(buffer.getPosition() + 1);
                                        }
                                        break;
                                        
                                    case 'D': // Left arrow
                                        if (buffer.getPosition() > 0) {
                                            write(STDOUT_FILENO, "\b", 1);
                                            const_cast<Buffer&>(buffer).setPosition(buffer.getPosition() - 1);
                                        }
                                        break;
                                        
                                    case 'H': // Home
                                        {
                                            size_t pos = buffer.getPosition();
                                            for (size_t i = 0; i < pos; ++i) {
                                                write(STDOUT_FILENO, "\b", 1);
                                            }
                                            const_cast<Buffer&>(buffer).setPosition(0);
                                        }
                                        break;
                                        
                                    case 'F': // End
                                        {
                                            size_t pos = buffer.getPosition();
                                            size_t len = buffer.getLength();
                                            for (size_t i = pos; i < len; ++i) {
                                                write(STDOUT_FILENO, "\033[C", 3);
                                            }
                                            const_cast<Buffer&>(buffer).setPosition(len);
                                        }
                                        break;
                                        
                                    case '1': // Home (some terminals)
                                        if (read(STDIN_FILENO, &seq[2], 1) == 1 && seq[2] == '~') {
                                            size_t pos = buffer.getPosition();
                                            for (size_t i = 0; i < pos; ++i) {
                                                write(STDOUT_FILENO, "\b", 1);
                                            }
                                            const_cast<Buffer&>(buffer).setPosition(0);
                                        }
                                        break;
                                        
                                    case '4': // End (some terminals)
                                        if (read(STDIN_FILENO, &seq[2], 1) == 1 && seq[2] == '~') {
                                            size_t pos = buffer.getPosition();
                                            size_t len = buffer.getLength();
                                            for (size_t i = pos; i < len; ++i) {
                                                write(STDOUT_FILENO, "\033[C", 3);
                                            }
                                            const_cast<Buffer&>(buffer).setPosition(len);
                                        }
                                        break;
                                        
                                    case '3': // Delete
                                        if (read(STDIN_FILENO, &seq[2], 1) == 1 && seq[2] == '~') {
                                            if (buffer.getPosition() < buffer.getLength()) {
                                                std::string value = buffer.getValue();
                                                value.erase(buffer.getPosition(), 1);
                                                buffer.setValue(value);
                                                
                                                // Redraw from cursor to end
                                                std::string remaining = value.substr(buffer.getPosition());
                                                remaining += " ";  // Clear last char
                                                write(STDOUT_FILENO, remaining.c_str(), remaining.length());
                                                
                                                // Move cursor back
                                                for (size_t i = 0; i < remaining.length() - 1; ++i) {
                                                    write(STDOUT_FILENO, "\b", 1);
                                                }
                                            }
                                        }
                                        break;
                                }
                            }
                        } else if (seq[0] == 'O') {
                            if (read(STDIN_FILENO, &seq[1], 1) == 1) {
                                switch (seq[1]) {
                                    case 'H': // Home
                                        {
                                            size_t pos = buffer.getPosition();
                                            for (size_t i = 0; i < pos; ++i) {
                                                write(STDOUT_FILENO, "\b", 1);
                                            }
                                            const_cast<Buffer&>(buffer).setPosition(0);
                                        }
                                        break;
                                        
                                    case 'F': // End
                                        {
                                            size_t pos = buffer.getPosition();
                                            size_t len = buffer.getLength();
                                            for (size_t i = pos; i < len; ++i) {
                                                write(STDOUT_FILENO, "\033[C", 3);
                                            }
                                            const_cast<Buffer&>(buffer).setPosition(len);
                                        }
                                        break;
                                }
                            }
                        }
                    }
                }
                return 0;
                
            default:
                // Regular character input
                if (c >= 32 && c <= 126) {  // Printable ASCII
                    // Insert character at cursor position
                    std::string value = buffer.getValue();
                    value.insert(buffer.getPosition(), 1, c);
                    buffer.setValue(value);
                    
                    // Write character and any remaining text
                    std::string remaining = value.substr(buffer.getPosition());
                    write(STDOUT_FILENO, remaining.c_str(), remaining.length());
                    
                    // Move cursor to correct position
                    const_cast<Buffer&>(buffer).setPosition(buffer.getPosition() + 1);
                    for (size_t i = 1; i < remaining.length(); ++i) {
                        write(STDOUT_FILENO, "\b", 1);
                    }
                }
                return 0;
        }
    }

}