/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Termcaps.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 00:00:00 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/13 22:07:58 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Readinput/Termcaps.hpp"
#include "Readinput/ReadInput.hpp"
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <sys/ioctl.h>
#include <termios.h>
#include <termcap.h>

// Terminal structure definition
struct TerminalInfo {
    struct termios term;
    size_t rows;
    size_t cols;
};

// TODO: This should be properly integrated later
struct OptionsInfo {
    bool multiwidth_chars = false;
};

// Global terminal info - this should be encapsulated later
static TerminalInfo terminal __attribute__((unused));
static OptionsInfo options __attribute__((unused));

namespace Terminal {

    // Static variables for cursor position
    static size_t current_row = 0;
    static size_t current_col = 0;

    size_t Termcaps::getCharSize(unsigned char c) {
        if (c >= 0xF0) return 4;
        if (c >= 0xE0) return 3;
        if (c >= 0xC0) return 2;
        return 1;
    }

    // Helper function to get character length
    static int getCharLength(unsigned char c) {
        if (c >= 0xF0) return 4;  // 4-byte
        if (c >= 0xE0) return 3;  // 3-byte
        if (c >= 0xC0) return 2;  // 2-byte
        if (c < 0x80)  return 1;  // 1-byte
        return 0;                 // Invalid byte
    }

    // Helper function to get codepoint
    static unsigned int getCharCodepoint(const std::string& value, size_t position, size_t length) {
        if (position >= value.length()) return 0;
        
        unsigned char c = value[position];
        if (c < 0x80) {
            return c;
        } else if (c < 0xE0 && length >= 2 && position + 1 < value.length()) {
            return ((c & 0x1F) << 6) | (value[position + 1] & 0x3F);
        } else if (c < 0xF0 && length >= 3 && position + 2 < value.length()) {
            return ((c & 0x0F) << 12) | ((value[position + 1] & 0x3F) << 6) | (value[position + 2] & 0x3F);
        } else if (c < 0xF8 && length >= 4 && position + 3 < value.length()) {
            return ((c & 0x07) << 18) | ((value[position + 1] & 0x3F) << 12) | 
                   ((value[position + 2] & 0x3F) << 6) | (value[position + 3] & 0x3F);
        }
        return 0;
    }

    size_t Termcaps::getCharWidth(size_t position, const std::string& value) {
        if (position >= value.length()) return 0;
        
        unsigned char c = value[position];

        // Control characters with zero width
        if (c == '\0' || c == '\a' || c == '\b' || c == '\f' || c == '\r' || c == '\v') {
            return 0;
        } else if (c == '\t') {
            return 8;
        }

        // Handle ANSI escape sequences
        if (c == '\033' && position + 1 < value.length()) {
            size_t i = position + 1;
            if (value[i] == '[') {
                while (++i < value.length()) {
                    char ch = value[i];
                    if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')) {
                        return 0;
                    }
                    if (!(ch >= '0' && ch <= '9') && ch != ';' && ch != '[') {
                        break;
                    }
                }
            }
            return 0;
        }

        unsigned int codepoint = getCharCodepoint(value, position, getCharLength(value[position]));
        
        // Wide characters (double-width)
        if ((codepoint >= 0x1100  && codepoint <= 0x115F)  ||  // Hangul Jamo
            (codepoint >= 0x2329  && codepoint <= 0x232A)  ||  // Angle brackets
            (codepoint >= 0x2E80  && codepoint <= 0x9FFF)  ||  // CJK Ideographs & Radicals
            (codepoint >= 0xAC00  && codepoint <= 0xD7A3)  ||  // Hangul syllables
            (codepoint >= 0xF900  && codepoint <= 0xFAFF)  ||  // CJK compatibility
            (codepoint >= 0xFE10  && codepoint <= 0xFE19)  ||  // Vertical forms
            (codepoint >= 0x1F300 && codepoint <= 0x1F64F) ||  // Emojis
            (codepoint >= 0x1F900 && codepoint <= 0x1F9FF)) {  // Supplemental Symbols
            return 2;
        }
        
        return 1;
    }

    size_t Termcaps::getCharsWidth(size_t from, size_t to, const std::string& value) {
        size_t total_chars = 0;

        if (from < to) {
            while (from < to) {
                total_chars += getCharWidth(from, value);
                do { 
                    from++; 
                } while (from < to && from < value.length() && (value[from] & 0xC0) == 0x80);
            }
        } else if (to < from) {
            while (to < from) {
                total_chars += getCharWidth(to, value);
                do { 
                    to++; 
                } while (to < from && to < value.length() && (value[to] & 0xC0) == 0x80);
            }
        }

        return total_chars;
    }

    size_t Termcaps::getPrevChar(size_t position, const std::string& value) {
        if (position > 0) {
            do { 
                position--; 
            } while (position > 0 && position < value.length() && (value[position] & 0xC0) == 0x80);
        }
        return position;
    }

    size_t Termcaps::getNoColorLength(const std::string& str) {
        if (str.empty()) {
            return 0;
        }

        size_t length = 0;
        size_t i = 0;
        
        while (i < str.length()) {
            if (str[i] == '\033') {
                i++;
                if (i < str.length() && str[i] == '[') {
                    while (i < str.length() && str[i] != 'm') {
                        i++;
                    }
                    if (i < str.length() && str[i] == 'm') {
                        i++;
                    }
                }
            } else {
                length++;
                i++;
            }
        }
        
        return length;
    }

    std::string Termcaps::removeColors(const std::string& str) {
        if (str.empty()) {
            return "";
        }

        size_t length = getNoColorLength(str);
        if (length == str.length()) {
            return str;
        }

        std::string result;
        result.reserve(length);
        
        size_t i = 0;
        while (i < str.length()) {
            if (str[i] == '\033') {
                i++;
                if (i < str.length() && str[i] == '[') {
                    while (i < str.length() && str[i] != 'm') {
                        i++;
                    }
                    if (i < str.length() && str[i] == 'm') {
                        i++;
                    }
                }
            } else {
                result += str[i++];
            }
        }

        return result;
    }

    void Termcaps::cursorUp() {
        cursorSet(current_row - 1, current_col);
    }

    void Termcaps::cursorDown() {
        cursorSet(current_row + 1, current_col);
    }

    void Termcaps::cursorLeft(int moves) {
        const auto& buffer = ReadInput::getBuffer();
        if (moves == 0) {
            moves = getCharWidth(buffer.getPosition(), buffer.getValue());
        }

        while (moves--) {
            if (current_col == 0) {
                cursorSet(current_row - 1, terminal.cols - 1);
            } else {
                cursorSet(current_row, current_col - 1);
            }
        }
    }

    void Termcaps::cursorRight(int moves) {
        const auto& buffer = ReadInput::getBuffer();
        if (moves == 0 && buffer.getPosition() == buffer.getLength()) {
            return;
        }
        if (moves == 0) {
            moves = getCharWidth(buffer.getPosition(), buffer.getValue());
        }

        while (moves--) {
            if (current_col >= terminal.cols - 1) {
                cursorSet(current_row + 1, 0);
            } else {
                cursorSet(current_row, current_col + 1);
            }
        }
    }

    void Termcaps::cursorGet() {
        char buf[32];
        memset(buf, 0, sizeof(buf));
        int a = 0, i = 0;
        char *action = tgetstr(const_cast<char*>("u7"), NULL);

        if (action) {
            write(STDIN_FILENO, action, strlen(action));
            read(STDIN_FILENO, buf, sizeof(buf) - 1);

            while (buf[i]) {
                if (buf[i] >= '0' && buf[i] <= '9') {
                    int start = i;
                    while (buf[i] >= '0' && buf[i] <= '9') i++;
                    char temp = buf[i];
                    buf[i] = '\0'; // Temporarily null-terminate for conversion
                    int value = std::atoi(&buf[start]);
                    buf[i] = temp; // Restore character
                    if (a++ == 0) {
                        current_row = value - 1;
                    } else {
                        current_col = value - 1;
                    }
                }
                i++;
            }
        }
    }

    void Termcaps::cursorSet(size_t new_row, size_t new_col) {
        if (new_row >= terminal.rows) {
            new_row = terminal.rows - 1;
        }
        
        char *action = tgetstr(const_cast<char*>("cm"), NULL);
        if (action) {
            action = tgoto(action, new_col, new_row);
            write(STDOUT_FILENO, action, strlen(action));
            current_row = new_row;
            current_col = new_col;
        }
    }

    void Termcaps::cursorMove(size_t from, size_t to) {
        const auto& buffer = ReadInput::getBuffer();
        if (from < to) {
            int total = getCharsWidth(from, to, buffer.getValue());
            if (total) cursorRight(total);
        } else if (from > to) {
            int total = getCharsWidth(to, from, buffer.getValue());
            if (total) cursorLeft(total);
        }
    }

    void Termcaps::cursorUpdate(size_t length) {
        const auto& buffer = ReadInput::getBuffer();
        cursorMove(buffer.getPosition(), length);
    }

    void Termcaps::cursorStartColumn() {
        cursorSet(current_row, 0);
    }

    void Termcaps::cursorHide() {
        char *action = tgetstr(const_cast<char*>("vi"), NULL);
        if (action) {
            write(STDOUT_FILENO, action, strlen(action));
        }
    }

    void Termcaps::cursorShow() {
        char *action = tgetstr(const_cast<char*>("ve"), NULL);
        if (action) {
            write(STDOUT_FILENO, action, strlen(action));
        }
    }

    int Termcaps::writeValue(int fd, const std::string& value) {
        return write(fd, value.c_str(), value.length());
    }

    int Termcaps::writeValue(int fd, const char* value, size_t length) {
        return write(fd, value, length);
    }

    int Termcaps::initialize() {
        // Get terminal capabilities
        char *term = getenv("TERM");
        if (!term) {
            term = const_cast<char*>("xterm");
        }

        static char termbuf[2048];
        if (tgetent(termbuf, term) < 1) {
            return -1;
        }

        // Get terminal size
        struct winsize ws;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
            terminal.rows = ws.ws_row;
            terminal.cols = ws.ws_col;
        } else {
            terminal.rows = 24;  // Default fallback
            terminal.cols = 80;
        }

        return 0;
    }

    void Termcaps::release() {
        // Cleanup terminal capabilities if needed
        cursorShow();
    }

}