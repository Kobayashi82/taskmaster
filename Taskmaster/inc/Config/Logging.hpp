/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logging.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/11 22:28:17 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/24 16:26:56 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include <fstream>															// std::ofstream
	#include <mutex>															// std::mutex

#pragma endregion

#pragma region "Enumarators"

	enum e_level { DEBUG, INFO, LOG, WARNING, ERROR, CRITICAL };

#pragma endregion

#pragma region "Tintin Reporter"

	class Tintin_reporter {

		private:

			//	Variables
			std::string		_logPath;											// Path to the log file
    		std::ofstream	_logFile;											// Output file stream for logging
			uint8_t			_logLevel;											// Current logging level
			std::mutex		_mutex;												// Mutex for thread-safe logging

			// Methods
			void			open();												// Opens or creates the log file in append mode
			void			clear();											// Clears all entries from the log
			void			rotateLog();										// Rotates log files
			void			createDirectory(const std::string& filePath);		// Creates the full directory path to the log file
			std::string		getTimestamp();										// Returns the current timestamp in log-friendly format

		public:

			//	Constructors
			Tintin_reporter(const std::string& logPath, uint8_t logLevel);		// Parameterized constructor
			Tintin_reporter(const Tintin_reporter& src);						// Copy constructor
			~Tintin_reporter();													// Destructor

			//	Overloads
			Tintin_reporter&	operator=(const Tintin_reporter& rhs);			// Assignment operator
			bool				operator==(const Tintin_reporter& rhs) const;	// Equality operator

			// Methods
			void			set_logLevel(uint8_t logLevel);						// Sets the logging level
			uint8_t			get_logLevel() const;								// Gets the current logging level
			std::string		get_logPath() const;								// Gets the log file path

			void			debug(const std::string& msg);						// Writes a DEBUG level log
    		void			info(const std::string& msg);						// Writes an INFO level log
			void			log(const std::string& msg);						// Writes a LOG level entry
			void			warning(const std::string& msg);					// Writes a WARNING level log
			void			error(const std::string& msg);						// Writes an ERROR level log
			void			critical(const std::string& msg);					// Writes a CRITICAL level log
	};

#pragma endregion

#pragma region "Variables"

	extern Tintin_reporter	*Log;												// Global pointer to Tintin_reporter

#pragma endregion
