/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Undo.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 00:00:00 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/13 22:08:02 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Readinput/ReadInput.hpp"
#include "Readinput/Termcaps.hpp"
#include <vector>
#include <string>
#include <unistd.h>

namespace Terminal {

    // Structure to hold undo state
    struct UndoState {
        std::string value;
        size_t position;
        
        UndoState(const std::string& val, size_t pos) : value(val), position(pos) {}
    };

    // Static variables for undo system
    static std::vector<UndoState> undo_stack;
    static std::vector<UndoState> redo_stack;
    static constexpr size_t MAX_UNDO_LEVELS = 100;

    void undoPushFunction(bool push) {
        if (!push) {
            return;
        }
        
        const auto& buffer = ReadInput::getBuffer();
        
        // Add current state to undo stack
        undo_stack.emplace_back(buffer.getValue(), buffer.getPosition());
        
        // Limit stack size
        if (undo_stack.size() > MAX_UNDO_LEVELS) {
            undo_stack.erase(undo_stack.begin());
        }
        
        // Clear redo stack when new action is performed
        redo_stack.clear();
    }

    void undoPopFunction() {
        if (undo_stack.empty()) {
            return; // Nothing to undo
        }
        
        auto& buffer = const_cast<Buffer&>(ReadInput::getBuffer());
        
        // Save current state to redo stack
        redo_stack.emplace_back(buffer.getValue(), buffer.getPosition());
        
        // Limit redo stack size
        if (redo_stack.size() > MAX_UNDO_LEVELS) {
            redo_stack.erase(redo_stack.begin());
        }
        
        // Restore previous state
        UndoState previous_state = undo_stack.back();
        undo_stack.pop_back();
        
        // Clear current line
        Termcaps::cursorMove(buffer.getPosition(), 0);
        std::string spaces(buffer.getLength(), ' ');
        write(STDOUT_FILENO, spaces.c_str(), spaces.length());
        Termcaps::cursorMove(buffer.getLength(), 0);
        
        // Set new state
        buffer.setValue(previous_state.value);
        buffer.setPosition(previous_state.position);
        
        // Redraw line
        if (!previous_state.value.empty()) {
            write(STDOUT_FILENO, previous_state.value.c_str(), previous_state.value.length());
            
            // Move cursor to correct position
            if (previous_state.position < previous_state.value.length()) {
                size_t moves = previous_state.value.length() - previous_state.position;
                for (size_t i = 0; i < moves; ++i) {
                    Termcaps::cursorLeft(1);
                }
            }
        }
    }

    void redoFunction() {
        if (redo_stack.empty()) {
            return; // Nothing to redo
        }
        
        auto& buffer = const_cast<Buffer&>(ReadInput::getBuffer());
        
        // Save current state to undo stack
        undo_stack.emplace_back(buffer.getValue(), buffer.getPosition());
        
        // Limit undo stack size
        if (undo_stack.size() > MAX_UNDO_LEVELS) {
            undo_stack.erase(undo_stack.begin());
        }
        
        // Restore next state
        UndoState next_state = redo_stack.back();
        redo_stack.pop_back();
        
        // Clear current line
        Termcaps::cursorMove(buffer.getPosition(), 0);
        std::string spaces(buffer.getLength(), ' ');
        write(STDOUT_FILENO, spaces.c_str(), spaces.length());
        Termcaps::cursorMove(buffer.getLength(), 0);
        
        // Set new state
        buffer.setValue(next_state.value);
        buffer.setPosition(next_state.position);
        
        // Redraw line
        if (!next_state.value.empty()) {
            write(STDOUT_FILENO, next_state.value.c_str(), next_state.value.length());
            
            // Move cursor to correct position
            if (next_state.position < next_state.value.length()) {
                size_t moves = next_state.value.length() - next_state.position;
                for (size_t i = 0; i < moves; ++i) {
                    Termcaps::cursorLeft(1);
                }
            }
        }
    }

    void undoAllFunction() {
        if (undo_stack.empty()) {
            return;
        }
        
        auto& buffer = const_cast<Buffer&>(ReadInput::getBuffer());
        
        // Save current state to redo stack
        redo_stack.emplace_back(buffer.getValue(), buffer.getPosition());
        
        // Go to the very first state
        UndoState first_state = undo_stack.front();
        
        // Move all undo states to redo stack (in reverse order)
        for (auto it = undo_stack.rbegin(); it != undo_stack.rend(); ++it) {
            if (it != undo_stack.rbegin()) { // Skip the last one (first state)
                redo_stack.push_back(*it);
            }
        }
        
        undo_stack.clear();
        
        // Clear current line
        Termcaps::cursorMove(buffer.getPosition(), 0);
        std::string spaces(buffer.getLength(), ' ');
        write(STDOUT_FILENO, spaces.c_str(), spaces.length());
        Termcaps::cursorMove(buffer.getLength(), 0);
        
        // Set first state
        buffer.setValue(first_state.value);
        buffer.setPosition(first_state.position);
        
        // Redraw line
        if (!first_state.value.empty()) {
            write(STDOUT_FILENO, first_state.value.c_str(), first_state.value.length());
            
            // Move cursor to correct position
            if (first_state.position < first_state.value.length()) {
                size_t moves = first_state.value.length() - first_state.position;
                for (size_t i = 0; i < moves; ++i) {
                    Termcaps::cursorLeft(1);
                }
            }
        }
    }

    void undoClearFunction() {
        undo_stack.clear();
        redo_stack.clear();
    }

}

// Update the ReadInput undo methods to use these implementations
namespace Terminal {
    void ReadInput::undoPush(bool push) {
        undoPushFunction(push);
    }

    void ReadInput::undoPop() {
        undoPopFunction();
    }

    void ReadInput::undoAll() {
        undoAllFunction();
    }

    void ReadInput::undoClear() {
        undoClearFunction();
    }
}