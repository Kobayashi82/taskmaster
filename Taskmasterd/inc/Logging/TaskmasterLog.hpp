/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TaskmasterLog.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/11 22:28:17 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/07 18:00:21 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include "Logging/LogRotate.hpp"

	#include <cstdint>															// uint8_t, uint16_t
	#include <string>															// std::string
	#include <map>																// std::map
	#include <deque>															// std::deque
	#include <fstream>															// std::ofstream

#pragma endregion

#pragma region "Log"

	class TaskmasterLog {

		private:

			// Variables
			std::string				_logfile;
			long					_logfile_maxbytes;
			uint16_t				_logfile_backups;
			uint8_t					_logfile_level;
			bool					_logfile_syslog;
			bool					_logfile_stdout;
			bool					_logfile_ready;
			std::deque<std::string>	_logfile_buffer;
			std::ofstream			_logfile_stream;
			size_t					_buffer_max_size;
			LogRotate				_logRotate;

			// Logging
			std::string	get_timestamp() const;
			void		add_buffer(const std::string& log);
			void		log(const std::string& msg, const std::string& level, bool add_level = true);


		public:

			// Constructors
			TaskmasterLog();
			TaskmasterLog(const std::string& logFile, size_t maxSize, uint16_t maxBackups, uint8_t level, bool syslog, bool _stdout, bool ready);
			TaskmasterLog(const TaskmasterLog&) = delete;
			TaskmasterLog(TaskmasterLog&&) = delete;
			~TaskmasterLog();

			// Overloads
			TaskmasterLog& operator=(const TaskmasterLog&) = delete;
			TaskmasterLog& operator=(TaskmasterLog&&) = delete;

			// Manage
			int						open();
			void					close();

			// Getters
			std::string				get_logfile() const;
			long					get_logfile_maxbytes() const;
			uint16_t				get_logfile_backups() const;
			uint8_t					get_logfile_level() const;
			bool					get_logfile_syslog() const;
			bool					get_logfile_stdout() const;
			bool					get_logfile_ready() const;
			size_t					get_buffer_size() const;
			size_t					get_buffer_max_size() const;
			std::deque<std::string>	get_log(size_t n = 20) const;

			// Setters
			void					set_logfile(const std::string& logfile);
			void					set_logfile_maxbytes(long logfile_maxbytes);
			void					set_logfile_backups(uint16_t logfile_backups);
			void					set_logfile_level(uint8_t logfile_level);
			void					set_logfile_syslog(bool logfile_syslog);
			void					set_logfile_stdout(bool logfile_stdout);
			void					set_logfile_ready(bool logfile_ready);
			void					set_buffer_max_size(size_t size);

			// Logging
			void					generic(const std::string& msg);
			void					debug(const std::string& msg);
    		void					info(const std::string& msg);
			void					warning(const std::string& msg);
			void					error(const std::string& msg);
			void					critical(const std::string& msg);

	};


#pragma endregion
	
#pragma region "Variables"

	extern TaskmasterLog Log;

#pragma endregion