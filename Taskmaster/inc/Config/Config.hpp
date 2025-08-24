/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/24 16:24:00 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/24 19:14:17 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <map>																	// std::map
#include <string>																// std::string
#include <cstdint>																// uint8_t

class Config {

	public:

		enum e_level { DEBUG, INFO, LOG, WARNING, ERROR, CRITICAL };

		struct s_taskmasterd {
			bool nodaemon;
			bool silent;
			std::string user;
			uint8_t umask;
			std::string directory;
			std::string logfile;
			size_t logfile_maxbytes;
			uint8_t logfile_backups;
			uint8_t loglevel;
			std::string pidfile;
			std::string identifier;
			std::string childlogdir;
			bool strip_ansi;
			bool nocleanup;
			uint16_t minfds;
			uint16_t minprocs;

			std::map<std::string, std::string> environment;

			s_taskmasterd();
		};

		static s_taskmasterd	taskmasterd;
		static bool				is_root;										// 

		static int	load_config(const std::string config_path);
		static void add_opt_args();
};
