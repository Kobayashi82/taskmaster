/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LogRotate.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/07 16:45:00 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/07 18:02:00 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include <string>														// std::string
	#include <cstdint>														// uint16_t
	#include <fstream>														// std::ofstream
	#include <functional>													// std::function

#pragma endregion

#pragma region "LogRotate"

	class LogRotate {

		private:

			// Variables
			std::string				_logPath;
			size_t					_maxSize;
			uint16_t				_maxBackups;
			std::function<void()>	_reopenCallback;

			// Methods
			void	clear_log();
			void	rotate_files();
			bool	should_rotate(std::ofstream& logFile) const;
			bool	should_rotate(const std::string& filePath) const;

		public:

			// Variables
			bool		enabled;

			// Constructors
			LogRotate();
			LogRotate(const std::string& logPath, size_t maxSize, uint16_t maxBackups);
			LogRotate(const LogRotate&) = delete;
			LogRotate(LogRotate&&) = delete;
			~LogRotate() = default;

			// Overloads
			LogRotate& operator=(const LogRotate&) = delete;
			LogRotate& operator=(LogRotate&&) = delete;

			// Getters
			std::string	get_LogPath() const;
			size_t		get_MaxSize() const;
			uint16_t	get_MaxBackups() const;

			// Setters
			void		set_LogPath(const std::string& logPath);
			void		set_MaxSize(size_t maxSize);
			void		set_MaxBackups(uint16_t maxBackups);
			void		set_ReopenCallback(std::function<void()> callback);

			// Methods
			void		rotate(std::ofstream& logFile);
			void		rotate(const std::string& filePath);

	};

#pragma endregion
