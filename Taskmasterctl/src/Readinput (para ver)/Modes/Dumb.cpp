/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Dumb.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 00:00:00 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/14 12:54:37 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Readline/ReadInput.hpp"
#include "Readline/History.hpp"
#include <unistd.h>

namespace Terminal {

    // Simple dumb terminal mode - minimal functionality
    int dumbMode(int readed) {
        auto& buffer = const_cast<Buffer&>(ReadInput::getBuffer());
        
        if (readed <= 0) {
            return 1; // Exit on EOF
        }

        unsigned char c = buffer.getChar();

        switch (c) {
            case 3:   // Ctrl+C
                buffer.clear();
                write(STDOUT_FILENO, "\n", 1);
                if (!ReadInput::getPrompt().empty()) {
                    write(STDOUT_FILENO, ReadInput::getPrompt().c_str(), ReadInput::getPrompt().length());
                }
                return 0;
            case 4:   // Ctrl+D (EOF)
                if (buffer.getLength() == 0) {
                    return 1; // Exit
                }
                return 0;
            case 10:  // Enter/Return
            case 13:
                if (!buffer.getValue().empty()) {
                    History::add(buffer.getValue());
                }
                write(STDOUT_FILENO, "\n", 1);
                return 1; // Signal completion
            case 127: // Backspace/Delete
                if (buffer.getPosition() > 0) {
                    buffer.deleteChar(buffer.getPosition() - 1);
                    // Simple backspace: move back, overwrite with space, move back again
                    write(STDOUT_FILENO, "\b \b", 3);
                }
                return 0;
            default:
                // Only accept printable ASCII characters
                if (c >= 32 && c <= 126) {
                    buffer.insertChar(c, buffer.getPosition());
                    write(STDOUT_FILENO, &c, 1);
                }
                return 0;
        }
    }

}

// Update the ReadInput::dumb method to use this implementation
namespace Terminal {
    int ReadInput::dumb(int readed) {
        return dumbMode(readed);
    }
}