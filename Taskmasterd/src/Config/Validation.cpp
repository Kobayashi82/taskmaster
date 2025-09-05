/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Validation.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 11:32:25 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/05 20:35:56 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Utils/Utils.hpp"
	#include "Config/Config.hpp"
	#include "Logging/TaskmasterLog.hpp"

	#include <unistd.h>															// getuid()
	#include <cstring>															// strerror()
	#include <unistd.h>															// gethostname()
	#include <iostream>															// std::cerr
	#include <filesystem>														// std::filesystem::path()
	#include <pwd.h>															// struct passwd, getpwuid()

#pragma endregion

	#pragma region "Options"

		int ConfigParser::validate_options(ConfigOptions& Options) const {
			static std::string	dir;
			std::string			errors;

			if (Options.options.find_first_of('d') != std::string::npos) {
				if (!Utils::valid_path(Options.directory, dir, true))
					errors += "directory:\t\tpath is invalid - " + std::string(strerror(errno)) + "\n";
				dir = Utils::expand_path(Options.directory, "", true, false);
			}

			if (Options.options.find_first_of('c') != std::string::npos) {
				if (!Utils::valid_path(Options.configuration, dir) || Utils::expand_path(Options.configuration, dir, true, false).empty())
					errors += "configuration:\t\tpath is invalid - " + std::string(strerror(errno)) + "\n";
			}

			if (Options.options.find_first_of('u') != std::string::npos) {
				if (!Utils::valid_user(Options.user))
					errors += "user:\t\t\tinvalid user\n";
			}

			if (Options.options.find_first_of('m') != std::string::npos) {
				if (!Utils::valid_umask(Options.umask))
					errors += "umask:\t\t\tmust be in octal format\n";
			}

			if (Options.options.find_first_of('l') != std::string::npos) {
				if (!Utils::valid_path(Options.logfile, dir, false, false, true) || Utils::expand_path(Options.logfile, dir).empty())
					errors += "logfile:\t\tpath is invalid - " + std::string(strerror(errno)) + "\n";
			}

			if (Options.options.find_first_of('y') != std::string::npos) {
				long bytes = Utils::parse_size(Options.logfile_maxbytes);
				if (bytes == -1 || !Utils::valid_number(std::to_string(bytes), 0, 1024 * 1024 * 1024))
					errors += "logfile_maxbytes:\tmust be a value between 0 bytes and 1024 MB\n";
			}

			if (Options.options.find_first_of('z') != std::string::npos) {
				if (!Utils::valid_number(Options.logfile_backups, 0, 1000))
					errors += "logfile_backups:\tmust be a value between 0 and 1000\n";
			}

			if (Options.options.find_first_of('e') != std::string::npos) {
				if (!Utils::valid_loglevel(Options.loglevel))
					errors += "loglevel:\t\tmust be one of: DEBUG, INFO, WARNING, ERROR, CRITICAL\n";
			}

			if (Options.options.find_first_of('j') != std::string::npos) {
				if (!Utils::valid_path(Options.pidfile, dir, false, false, true) || Utils::expand_path(Options.pidfile, dir).empty())
					errors += "pidfile:\t\tpath is invalid - " + std::string(strerror(errno)) + "\n";
			}

			if (Options.options.find_first_of('q') != std::string::npos) {
				if (!Utils::valid_path(Options.childlogdir, dir, true))
					errors += "childlogdir:\t\tpath is invalid - " + std::string(strerror(errno)) + "\n";
			}

			if (Options.options.find_first_of('a') != std::string::npos) {
				if (!Utils::valid_number(Options.minfds, 1, 65535))
					errors += "minfds:\t\t\tmust be a value between 1 and 65535\n";
			}

			if (Options.options.find_first_of('p') != std::string::npos) {
				if (!Utils::valid_number(Options.minprocs, 1, 65535))
					errors += "minprocs:\t\tmust be a value between 1 and 10000\n";
			}

			if (!errors.empty()) { std::cerr << Options.fullName << ": invalid options: \n\n" <<  errors; return (2); }

			return (0);
		}

	#pragma endregion

