/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 11:33:13 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/01 15:06:43 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Options.hpp"
	#include "Config/Parser.hpp"
	#include "Logging/TaskmasterLog.hpp"

	#include <cstring>															// strerror()
	#include <unistd.h>															// gethostname()
	#include <fstream>															// std::ifstream
	#include <iostream>															// std::cout()
	#include <algorithm>														// std::replace()

#pragma endregion

#pragma region "Parse"

	int ConfigParser::parse(const std::string& filePath) {
		std::string configFile = filePath;

		if (configFile.empty()) {
			const std::string candidates[] = {
				"../etc/taskmasterd.conf",
				"../taskmasterd.conf",
				"taskmasterd.conf",
				"etc/taskmasterd.conf",
				"/etc/taskmasterd.conf",
				"/etc/taskmaster/taskmasterd.conf"
			};

			for (const auto& path : candidates) {
				configFile = expand_path(path, "", true, false);
				if (!configFile.empty()) break;
			}
		} else configFile = expand_path(configFile, "", true, false);

		if (configFile.empty()) {
			Log.error("no configuration file found");
			Log.warning("tasksmasterd is running without a configuration file");
			return (0);
		}

		std::ifstream file(configFile);
		if (!file.is_open()) {
			Log.error("cannot open config file: " + configFile + " - " + strerror(errno));
			Log.warning("tasksmasterd is running without a configuration file");
			return (0);
		}

		currentSection.clear();
		sections.clear();
		errors.clear();
		order = 0;

		std::string	line;
		int			lineNumber = 0;
		bool		invalidSection = false;

		while (std::getline(file, line)) { lineNumber++; order++;
			line = trim(remove_comments(line));
			if (line.empty()) continue;

			if		(currentSection == "include" && is_section(line))	include_process(configFile);
			if		(is_section(line))									invalidSection = section_parse(line, lineNumber, configFile);
			else if	(!invalidSection)									key_parse(line, lineNumber, configFile);
		}

		if (currentSection == "include")								include_process(configFile);

		Parser.validate("", "", "");

		return (0);
	}

#pragma endregion

#pragma region "Merged Options"

	void ConfigParser::merge_options(ConfigOptions& Options) {
		if (Options.options.find_first_of('n') != std::string::npos) sections["taskmasterd"]["nodaemon"].value			= Options.nodaemon;
		if (Options.options.find_first_of('s') != std::string::npos) sections["taskmasterd"]["silent"].value			= Options.silent;
		if (Options.options.find_first_of('u') != std::string::npos) sections["taskmasterd"]["user"].value				= Options.user;
		if (Options.options.find_first_of('m') != std::string::npos) sections["taskmasterd"]["umask"].value				= Options.umask;
		if (Options.options.find_first_of('d') != std::string::npos) sections["taskmasterd"]["directory"].value			= Options.directory;
		if (Options.options.find_first_of('l') != std::string::npos) sections["taskmasterd"]["logfile"].value			= Options.logfile;
		if (Options.options.find_first_of('y') != std::string::npos) sections["taskmasterd"]["logfile_maxbytes"].value	= Options.logfile_maxbytes;
		if (Options.options.find_first_of('z') != std::string::npos) sections["taskmasterd"]["logfile_backups"].value	= Options.logfile_backups;
		if (Options.options.find_first_of('e') != std::string::npos) sections["taskmasterd"]["loglevel"].value			= Options.loglevel;
		if (Options.options.find_first_of('j') != std::string::npos) sections["taskmasterd"]["pidfile"].value			= Options.pidfile;
		if (Options.options.find_first_of('i') != std::string::npos) sections["taskmasterd"]["identifier"].value		= Options.identifier;
		if (Options.options.find_first_of('q') != std::string::npos) sections["taskmasterd"]["childlogdir"].value		= Options.childlogdir;
		if (Options.options.find_first_of('t') != std::string::npos) sections["taskmasterd"]["strip_ansi"].value		= Options.strip_ansi;
		if (Options.options.find_first_of('k') != std::string::npos) sections["taskmasterd"]["nocleanup"].value			= Options.nocleanup;
		if (Options.options.find_first_of('a') != std::string::npos) sections["taskmasterd"]["minfds"].value			= Options.minfds;
		if (Options.options.find_first_of('p') != std::string::npos) sections["taskmasterd"]["minprocs"].value			= Options.minprocs;
	}

#pragma endregion

#pragma region "Print"

	void ConfigParser::print() const {
		for (const auto& section : sections) {
			std::cout << "[" << section.first << "]" << std::endl;
			for (const auto& kv : section.second) {
				std::string value = kv.second.value;
				std::replace(value.begin(), value.end(), '\n', ',');
				std::cout << kv.first << " = " << value << std::endl;
			}
			std::cout << std::endl;
		}
	}

#pragma endregion
