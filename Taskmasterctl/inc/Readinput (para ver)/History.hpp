/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   History.hpp                                        :+:      :+:    :+:   */
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

	enum class HistoryType { 
		MEMORY, 
		FILE 
	};

	class HistoryEntry {
	private:
		std::string		line;
		void*			data;
		size_t			event;
		size_t			length;

	public:
		HistoryEntry(const std::string& line, void* data = nullptr);
		~HistoryEntry() = default;

		// Getters
		const std::string&	getLine() const { return line; }
		void*				getData() const { return data; }
		size_t				getEvent() const { return event; }
		size_t				getLength() const { return length; }

		// Setters
		void				setLine(const std::string& new_line);
		void				setData(void* new_data) { data = new_data; }
		void				setEvent(size_t new_event) { event = new_event; }
		void				setLength(size_t new_length) { length = new_length; }
	};

	class History {
	private:
		static std::vector<std::unique_ptr<HistoryEntry>>	entries;
		static std::string									filename;
		static size_t										memory_size;
		static size_t										file_size;
		static size_t										current_position;
		static size_t										current_event;
		static bool											last_added;

	public:
		History() = delete;  // Static class
		~History() = delete;

		// File operations
		static void			setFilename(const std::string& name);
		static int			read(const std::string& filename);
		static int			write(const std::string& filename);

		// Size management
		static size_t		getSize(HistoryType type);
		static void			setSize(size_t value, HistoryType type);
		static void			unsetSize(HistoryType type);

		// Entry management
		static int			add(const std::string& line, bool force = false);
		static int			replace(size_t pos, const std::string& line, void* data = nullptr);
		static void			removeOffset(int offset);
		static void			remove(size_t pos);
		static void			removeEvent(size_t event);
		static void			removeCurrent(bool remove_event = false);
		static void			removeLastIfAdded(bool remove_event = false);
		static void			clear();

		// Retrieval
		static std::vector<std::unique_ptr<HistoryEntry>>	clone();
		static size_t										length();
		static HistoryEntry*								get(size_t pos);
		static HistoryEntry*								current();
		static HistoryEntry*								getByEvent(size_t event);
		static HistoryEntry*								getLastIfAdded();

		// Navigation
		static std::string	prev();
		static std::string	next();
		static size_t		getPosition();
		static int			getEventPosition(size_t event);
		static void			setPosition(size_t pos);
		static void			setPositionEnd();

		// Display
		static int			print(size_t offset = 0, bool hide_events = false);

		// Initialization
		static int			initialize();

		// Cleanup
		static void			cleanup();
	};

}