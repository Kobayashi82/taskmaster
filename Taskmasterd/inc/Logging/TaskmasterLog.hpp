/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TaskmasterLog.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/11 22:28:17 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/04 12:45:50 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

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

			std::string	getTimestamp() const;

		public:

			// Constructors
			TaskmasterLog() = default;
			TaskmasterLog(const TaskmasterLog&) = delete;
			~TaskmasterLog() = default;

			// Overloads
			TaskmasterLog& operator=(const TaskmasterLog&) = delete;

			int				open();

			// Getters
			std::string		get_logfile() const;
			long			get_logfile_maxbytes() const;
			uint16_t		get_logfile_backups() const;
			uint8_t			get_logfile_level() const;
			bool			get_logfile_syslog() const;
			bool			get_logfile_stdout() const;
			bool			get_logfile_ready() const;
			
			// Setters
			void			set_logfile(const std::string& logfile);
			void			set_logfile_maxbytes(std::string logfile_maxbytes);
			void			set_logfile_maxbytes(long logfile_maxbytes);
			void			set_logfile_backups(std::string logfile_backups);
			void			set_logfile_backups(uint16_t logfile_backups);
			void			set_logfile_level(std::string logfile_level);
			void			set_logfile_level(uint8_t logfile_level);
			void			set_logfile_syslog(std::string logfile_syslog);
			void			set_logfile_syslog(bool logfile_syslog);
			void			set_logfile_stdout(std::string logfile_stdout);
			void			set_logfile_stdout(bool logfile_stdout);
			void			set_logfile_ready(std::string logfile_ready);
			void			set_logfile_ready(bool logfile_ready);

			// Log
			void			log(const std::string& msg, const std::string& level, bool add_level = true);
			void			generic(const std::string& msg);
			void			debug(const std::string& msg);
    		void			info(const std::string& msg);
			void			warning(const std::string& msg);
			void			error(const std::string& msg);
			void			critical(const std::string& msg);

	};


#pragma endregion
	
#pragma region "Variables"

	extern TaskmasterLog Log;

#pragma endregion