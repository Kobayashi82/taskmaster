/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 11:33:13 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/02 21:09:52 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Options.hpp"
	#include "Config/Config.hpp"
	#include "Logging/TaskmasterLog.hpp"

	#include <unistd.h>															// getuid()
	#include <cstring>															// strerror()
	#include <fstream>															// std::ifstream
	#include <iostream>															// std::cout()
	#include <algorithm>														// std::replace()

#pragma endregion

#pragma region "Parse"

	void ConfigParser::parse(const std::string& filePath) {
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
			return ;
		}

		std::ifstream file(configFile);
		if (!file.is_open()) {
			Log.error("cannot open config file: " + configFile + " - " + strerror(errno));
			Log.warning("tasksmasterd is running without a configuration file");
			return ;
		}

		currentSection.clear();
		sections.clear();
		errors.clear();
		error_maxLevel = 0;
		order = 0;

		std::string	line;
		int			lineNumber = 0;
		bool		invalidSection = false;

		while (std::getline(file, line)) { lineNumber++; order += 2;
			line = trim(remove_comments(line));
			if (line.empty()) continue;

			if		(currentSection == "include" && is_section(line))	include_process(configFile);
			if		(is_section(line))									invalidSection = section_parse(line, lineNumber, configFile);
			else if	(!invalidSection)									key_parse(line, lineNumber, configFile);
		}

		if (currentSection == "include")								include_process(configFile);

		return ;
	}

#pragma endregion

#pragma region "Merged Options"

	void ConfigParser::merge_options(const ConfigOptions& Options) {
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

#pragma region "Load"

	int ConfigParser::load(int argc, char **argv) {
		int result = 0;

		is_root = getuid() == 0;

		ConfigOptions Options;
		if ((result = Options.parse(argc, argv))) return (result);

		if (Options.configuration.empty() && is_root) {
			Log.warning("taskmasterd is running as root and it is searching for its configuration file in default locations (including its current working directory). You probably want to specify a \"-c\" argument specifying an absolute path to a configuration file for improved security.");
		}
		parse(Options.configuration);
		merge_options(Options);

		validate();
		error_print();

		if (errors.size())	{
			if (error_maxLevel == WARNING)	  Log.warning	("configuration loaded with warnings. Review recommended");
			if (error_maxLevel == ERROR)	  Log.error		("configuration loaded with errors. Execution can continue");
			if (error_maxLevel == CRITICAL)	{ Log.critical	("configuration loaded with critical errors. Execution aborted"); result = 2; }
		}
		else Log.info("configuration loaded succesfully");

		Log.set_logfile_stdout(parse_bool(get_value("taskmasterd", "nodaemon")) && !parse_bool(get_value("taskmasterd", "silent")));
		Log.set_logfile(get_value("taskmasterd", "logfile"));
		Log.set_logfile_ready(true);

		return(result);
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
