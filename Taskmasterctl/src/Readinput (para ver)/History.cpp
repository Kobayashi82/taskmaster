/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   History.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 00:00:00 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/14 14:36:52 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Readline/History.hpp"
#include <fstream>
#include <algorithm>
#include <climits>
#include <unistd.h>
#include <iostream>

// TODO: These should be properly integrated later
struct OptionsInfo {
    bool multiwidth_chars = false;
};

struct ProjectInfo {
    std::string name = "taskmaster";
};

// Temporary global variables
static OptionsInfo options __attribute__((unused));
static ProjectInfo project __attribute__((unused));

namespace Terminal {

    // Static member definitions
    std::vector<std::unique_ptr<HistoryEntry>> History::entries;
    std::string History::filename;
    size_t History::memory_size = 1000;
    size_t History::file_size = 2000;
    size_t History::current_position = 0;
    size_t History::current_event = 1;
    bool History::last_added = false;

    constexpr size_t HIST_MAXSIZE = 5000;

    // HistoryEntry class implementation
    HistoryEntry::HistoryEntry(const std::string& line, void* data) 
        : line(line), data(data), event(0), length(line.length()) {
    }

    void HistoryEntry::setLine(const std::string& new_line) {
        line = new_line;
        length = line.length();
    }

    // History class implementation
    void History::setFilename(const std::string& name) {
        filename = name;
    }

    size_t History::getSize(HistoryType type) {
        switch (type) {
            case HistoryType::MEMORY:
                return memory_size;
            case HistoryType::FILE:
                return file_size;
            default:
                return memory_size;
        }
    }

    void History::setSize(size_t value, HistoryType type) {
        size_t new_size = std::min(std::max(static_cast<size_t>(0), value), HIST_MAXSIZE);

        switch (type) {
            case HistoryType::FILE:
                file_size = new_size;
                break;
            case HistoryType::MEMORY:
                memory_size = new_size;
                if (memory_size < entries.size()) {
                    // Keep only the most recent entries
                    auto start_it = entries.begin() + (entries.size() - memory_size);
                    std::vector<std::unique_ptr<HistoryEntry>> new_entries;
                    new_entries.reserve(memory_size);
                    
                    for (auto it = start_it; it != entries.end(); ++it) {
                        new_entries.push_back(std::move(*it));
                    }
                    
                    entries = std::move(new_entries);
                    current_position = entries.size() > 0 ? entries.size() - 1 : 0;
                }
                break;
        }
    }

    void History::unsetSize(HistoryType type) {
        switch (type) {
            case HistoryType::MEMORY:
                memory_size = INT_MAX;
                break;
            case HistoryType::FILE:
                file_size = INT_MAX;
                break;
        }
    }

    int History::read(const std::string& filename_param) {
        std::string file_to_read = filename_param.empty() ? filename : filename_param;
        
        if (file_to_read.empty() || access(file_to_read.c_str(), R_OK) != 0) {
            return 1;
        }

        if (memory_size == 0) {
            return 0;
        }

        std::ifstream file(file_to_read);
        if (!file.is_open()) {
            return 1;
        }

        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty()) {
                add(line, false);
            }
        }

        file.close();
        return 0;
    }

    int History::write(const std::string& filename_param) {
        std::string file_to_write = filename_param.empty() ? filename : filename_param;
        
        if (file_to_write.empty()) {
            return 1;
        }

        std::ofstream file(file_to_write);
        if (!file.is_open()) {
            return 1;
        }

        size_t start_index = 0;
        if (file_size != INT_MAX && entries.size() > file_size) {
            start_index = entries.size() - file_size;
        }

        for (size_t i = start_index; i < entries.size(); ++i) {
            file << entries[i]->getLine() << std::endl;
        }

        file.close();
        return 0;
    }

    int History::add(const std::string& line, bool force) {
        if (line.empty()) {
            return 1;
        }

        // Check for duplicates if not forced
        if (!force && !entries.empty()) {
            const auto& last_entry = entries.back();
            if (last_entry->getLine() == line) {
                return 0; // Duplicate, don't add
            }
        }

        // Check memory size limit
        if (memory_size != INT_MAX && entries.size() >= memory_size) {
            // Remove oldest entry
            entries.erase(entries.begin());
        }

        // Create new entry
        auto new_entry = std::make_unique<HistoryEntry>(line);
        new_entry->setEvent(current_event++);
        
        entries.push_back(std::move(new_entry));
        current_position = entries.size();
        last_added = true;

        return 0;
    }

    int History::replace(size_t pos, const std::string& line, void* data) {
        if (pos >= entries.size()) {
            return 1;
        }

        entries[pos]->setLine(line);
        entries[pos]->setData(data);
        return 0;
    }

    void History::removeOffset(int offset) {
        if (offset == 0 || entries.empty()) {
            return;
        }

        if (offset > 0) {
            // Remove from end
            size_t to_remove = std::min(static_cast<size_t>(offset), entries.size());
            entries.erase(entries.end() - to_remove, entries.end());
        } else {
            // Remove from beginning
            size_t to_remove = std::min(static_cast<size_t>(-offset), entries.size());
            entries.erase(entries.begin(), entries.begin() + to_remove);
        }

        // Adjust current position
        if (current_position >= entries.size()) {
            current_position = entries.size();
        }
    }

    void History::remove(size_t pos) {
        if (pos < entries.size()) {
            entries.erase(entries.begin() + pos);
            if (current_position > pos) {
                current_position--;
            } else if (current_position >= entries.size()) {
                current_position = entries.size();
            }
        }
    }

    void History::removeEvent(size_t event) {
        auto it = std::find_if(entries.begin(), entries.end(),
            [event](const std::unique_ptr<HistoryEntry>& entry) {
                return entry->getEvent() == event;
            });
        
        if (it != entries.end()) {
            size_t pos = std::distance(entries.begin(), it);
            remove(pos);
        }
    }

    void History::removeCurrent(bool remove_event) {
        if (current_position < entries.size()) {
            if (remove_event) {
                removeEvent(entries[current_position]->getEvent());
            } else {
                remove(current_position);
            }
        }
    }

    void History::removeLastIfAdded(bool remove_event) {
        if (last_added && !entries.empty()) {
            if (remove_event) {
                removeEvent(entries.back()->getEvent());
            } else {
                entries.pop_back();
            }
            last_added = false;
        }
    }

    void History::clear() {
        entries.clear();
        current_position = 0;
        current_event = 1;
        last_added = false;
    }

    std::vector<std::unique_ptr<HistoryEntry>> History::clone() {
        std::vector<std::unique_ptr<HistoryEntry>> cloned;
        cloned.reserve(entries.size());
        
        for (const auto& entry : entries) {
            cloned.push_back(std::make_unique<HistoryEntry>(
                entry->getLine(), entry->getData()));
        }
        
        return cloned;
    }

    size_t History::length() {
        return entries.size();
    }

    HistoryEntry* History::get(size_t pos) {
        if (pos < entries.size()) {
            return entries[pos].get();
        }
        return nullptr;
    }

    HistoryEntry* History::current() {
        if (current_position < entries.size()) {
            return entries[current_position].get();
        }
        return nullptr;
    }

    HistoryEntry* History::getByEvent(size_t event) {
        auto it = std::find_if(entries.begin(), entries.end(),
            [event](const std::unique_ptr<HistoryEntry>& entry) {
                return entry->getEvent() == event;
            });
        
        return (it != entries.end()) ? it->get() : nullptr;
    }

    HistoryEntry* History::getLastIfAdded() {
        if (last_added && !entries.empty()) {
            return entries.back().get();
        }
        return nullptr;
    }

    std::string History::prev() {
        if (entries.empty()) {
            return "";
        }

        if (current_position > 0) {
            current_position--;
        }
        
        return entries[current_position]->getLine();
    }

    std::string History::next() {
        if (entries.empty()) {
            return "";
        }

        if (current_position < entries.size() - 1) {
            current_position++;
            return entries[current_position]->getLine();
        } else {
            current_position = entries.size();
            return "";
        }
    }

    size_t History::getPosition() {
        return current_position;
    }

    int History::getEventPosition(size_t event) {
        for (size_t i = 0; i < entries.size(); ++i) {
            if (entries[i]->getEvent() == event) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    void History::setPosition(size_t pos) {
        current_position = std::min(pos, entries.size());
    }

    void History::setPositionEnd() {
        current_position = entries.size();
    }

    int History::print(size_t offset, bool hide_events) {
        if (entries.empty()) {
            return 0;
        }

        size_t start = std::min(offset, entries.size());
        
        for (size_t i = start; i < entries.size(); ++i) {
            if (hide_events) {
                std::cout << entries[i]->getLine() << std::endl;
            } else {
                std::cout << entries[i]->getEvent() << " " 
                         << entries[i]->getLine() << std::endl;
            }
        }

        return 0;
    }

    int History::initialize() {
        clear();
        
        // Try to load from default history file
        if (!filename.empty()) {
            read(filename);
        }
        
        return 0;
    }

    void History::cleanup() {
        // Save to file if filename is set
        if (!filename.empty()) {
            write(filename);
        }
        
        clear();
    }

}