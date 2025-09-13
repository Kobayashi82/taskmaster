/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ReadInput.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 00:00:00 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/13 22:13:14 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Readinput/ReadInput.hpp"
#include "Readinput/Termcaps.hpp"
#include "Readinput/Prompt.hpp"
#include "Readinput/History.hpp"
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <iostream>
#include <cstring>

// TODO: These will need to be properly defined or removed
struct TerminalInfo {
    struct termios term;
    size_t rows;
    size_t cols;
};

struct OptionsInfo {
    bool emacs = true;
    bool multiwidth_chars = false;
};

// Temporary global variables - these should be properly integrated later
static TerminalInfo terminal __attribute__((unused));
static OptionsInfo options __attribute__((unused));

namespace Terminal {

    // Static member definitions
    Buffer ReadInput::buffer;
    std::string ReadInput::term_prompt;
    bool ReadInput::raw_mode = false;
    bool ReadInput::hist_searching = false;
    InputMode ReadInput::input_mode = InputMode::READLINE;

    // Buffer class implementation
    Buffer::Buffer() : c(0), position(0), length(0), capacity(1024),
                      shift_pressed(false), alt_pressed(false), ctrl_pressed(false) {
        value.reserve(capacity);
    }

    void Buffer::setValue(const std::string& val) {
        value = val;
        length = val.length();
        if (position > length) {
            position = length;
        }
    }

    void Buffer::setPosition(size_t pos) {
        if (pos <= length) {
            position = pos;
        }
    }

    void Buffer::setCapacity(size_t cap) {
        if (cap > capacity) {
            capacity = cap;
            value.reserve(capacity);
        }
    }

    void Buffer::clear() {
        value.clear();
        position = 0;
        length = 0;
        c = 0;
        shift_pressed = false;
        alt_pressed = false;
        ctrl_pressed = false;
    }

    void Buffer::resize(size_t new_size) {
        if (new_size > capacity) {
            capacity = new_size;
            value.reserve(capacity);
        }
    }

    void Buffer::insertChar(char ch, size_t pos) {
        if (pos <= length) {
            value.insert(pos, 1, ch);
            length++;
            if (position >= pos) {
                position++;
            }
        }
    }

    void Buffer::deleteChar(size_t pos) {
        if (pos < length) {
            value.erase(pos, 1);
            length--;
            if (position > pos) {
                position--;
            } else if (position == length && position > 0) {
                position--;
            }
        }
    }

    void Buffer::insertString(const std::string& str, size_t pos) {
        if (pos <= length) {
            value.insert(pos, str);
            length += str.length();
            if (position >= pos) {
                position += str.length();
            }
        }
    }

    void Buffer::deleteRange(size_t start, size_t end) {
        if (start < end && start < length) {
            if (end > length) {
                end = length;
            }
            value.erase(start, end - start);
            length -= (end - start);
            if (position >= end) {
                position -= (end - start);
            } else if (position > start) {
                position = start;
            }
        }
    }

    // ReadInput class implementation
    void ReadInput::disableRawMode() {
        if (raw_mode) {
            raw_mode = false;
            Termcaps::cursorShow();
            Termcaps::release();
            
            if (fcntl(STDIN_FILENO, F_GETFD) == -1) {
                int tty_fd = open("/dev/tty", O_RDWR);
                if (tty_fd == -1) {
                    buffer.clear();
                    write(STDERR_FILENO, "\n", 1);
                    // exit_error(STDIN_CLOSED, 1, NULL, true); // Will need to adapt this
                    exit(1);
                }
                tcsetattr(tty_fd, TCSAFLUSH, &terminal.term);
                dup2(tty_fd, STDIN_FILENO);
                close(tty_fd);
                write(STDOUT_FILENO, "\n", 1);
            } else {
                tcsetattr(STDIN_FILENO, TCSAFLUSH, &terminal.term);
            }
            term_prompt.clear();
        }
    }

    void ReadInput::enableRawMode() {
        raw_mode = true;
        tcgetattr(STDIN_FILENO, &terminal.term);
        Termcaps::initialize();

        struct termios raw = terminal.term;
        raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
        raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
        raw.c_oflag &= ~(OPOST);
        raw.c_cc[VMIN] = 1;
        raw.c_cc[VTIME] = 0;

        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    }

    int ReadInput::checkTTY() {
        if (fcntl(STDIN_FILENO, F_GETFD) == -1) {
            int tty_fd = open("/dev/tty", O_RDWR);
            if (tty_fd == -1) {
                buffer.clear();
                disableRawMode();
                write(STDERR_FILENO, "\n", 1);
                // exit_error(STDIN_CLOSED, 1, NULL, true); // Will need to adapt this
                exit(1);
            }
            dup2(tty_fd, STDIN_FILENO);
            close(tty_fd);
            return 1;
        }
        
        if (fcntl(STDOUT_FILENO, F_GETFD) == -1) {
            int tty_fd = open("/dev/tty", O_WRONLY);
            if (tty_fd == -1) {
                buffer.clear();
                disableRawMode();
                write(STDERR_FILENO, "\n", 1);
                // exit_error(STDOUT_CLOSED, 1, NULL, true); // Will need to adapt this
                exit(1);
            }
            dup2(tty_fd, STDOUT_FILENO);
            close(tty_fd);
        }

        return 0;
    }

    std::string ReadInput::readInput(const std::string& prompt) {
        int result = 0;
        buffer.clear();
        buffer.setCapacity(1024);
        buffer.setCtrlPressed(false);
        buffer.setAltPressed(false);
        buffer.setShiftPressed(false);

        enableRawMode();

        term_prompt = prompt;
        if (!term_prompt.empty()) {
            write(STDOUT_FILENO, term_prompt.c_str(), term_prompt.length());
        }

        Termcaps::cursorGet();
        while (!result) {
            Termcaps::cursorShow();
            unsigned char input_char;
            int readed = read(STDIN_FILENO, &input_char, 1);
            buffer.setChar(input_char);
            if (checkTTY()) continue;
            Termcaps::cursorHide();

            if (hist_searching && historySearch()) continue;

            // Check options for input mode - will need to adapt this
            if (options.emacs) {
                input_mode = InputMode::READLINE;
                result = readline(readed);
            } else {
                input_mode = InputMode::DUMB;
                result = dumb(readed);
            }
        }
        
        History::setPositionEnd();
        undoClear();
        
        disableRawMode();
        
        return buffer.getValue();
    }

    std::string ReadInput::getInput() {
        return buffer.getValue();
    }

    void ReadInput::cleanup() {
        if (raw_mode) {
            disableRawMode();
        }
        buffer.clear();
        term_prompt.clear();
        hist_searching = false;
    }

    // Declare the external readline function
    int readlineMode(int readed);

    int ReadInput::readline(int readed) {
        return readlineMode(readed);
    }

}