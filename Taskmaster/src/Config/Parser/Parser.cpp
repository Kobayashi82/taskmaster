/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 11:33:13 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/30 21:21:57 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Options.hpp"
	#include "Config/Parser.hpp"

	#include <unistd.h>															// gethostname()
	#include <fstream>															// std::ifstream
	#include <iostream>															// std::cout

#pragma endregion

#pragma region "Parse"

	void ConfigParser::parse(const std::string& filePath) {
		std::string ConfigFile = filePath;

		if (ConfigFile.empty()) {
			const std::string candidates[] = {
				"../etc/taskmasterd.conf",
				"../taskmasterd.conf",
				"taskmasterd.conf",
				"etc/taskmasterd.conf",
				"/etc/taskmasterd.conf",
				"/etc/taskmaster/taskmasterd.conf"
			};

			for (const auto& path : candidates) {
				ConfigFile = expand_path(path, "", true);
				if (!ConfigFile.empty() && std::filesystem::exists(ConfigFile)) break;
				ConfigFile = "";
			}
		}

		if (ConfigFile.empty()) throw std::runtime_error("No configuration file found\n");

		configPath = expand_path(ConfigFile);
		if (configPath.empty()) configPath = ConfigFile;

		std::ifstream file(configPath);
		if (!file.is_open()) throw std::runtime_error("Cannot open config file: " + ConfigFile + "\n");

		char hostname[255];
		environment_add(environment_config, "HOST_NAME", (!gethostname(hostname, sizeof(hostname))) ? std::string(hostname) : "unknown");
		environment_add(environment_config, "HERE", configPath.parent_path());
		environment_initialize(environment);

		currentSection.clear();
		sections.clear();

		std::string	line, errors;
		int			lineNumber = 0;
		bool		invalid_section = false;

		while (std::getline(file, line)) { lineNumber++;
			line = trim(key_remove_comments(line));
			if (line.empty()) continue;

			try {
				if (currentSection == "include" && is_section(line)) {
					try { include_process(); }
					catch (const std::exception& e) {
						errors += ((errors.empty()) ? "" : "\n") + std::string(e.what());
						section_on_error = true;
					}
					environment_add(environment_config, "HERE", configPath.parent_path());
				}
				if		(is_section(line))	{ section_parse(line); invalid_section = false; }
				else if	(invalid_section)	  continue;
				else						  key_parse(line);
			}
			catch (const std::exception& e) {
				if (std::string(e.what()).find("ignore section") != std::string::npos)		{ invalid_section = true; continue; }
				if (std::string(e.what()).find("invalid section") != std::string::npos)		  invalid_section = true;
				if (std::string(e.what()).find("duplicate section") != std::string::npos)	  invalid_section = true;
				if (section_on_error) {
					errors += ((errors.empty()) ? "" : "\n") + std::string("[" + configPath.string() + "]\n");
					section_on_error = false;
				}
				errors += "Error at line " + std::to_string(lineNumber) + ":\t" + e.what() + "\n";
			}
		}

		if (currentSection == "include") {
			try { include_process(); }
			catch (const std::exception& e) {
				errors += ((errors.empty()) ? "" : "\n") + std::string(e.what());
			}
		}

		if (!errors.empty()) throw std::runtime_error(errors);
	}

#pragma endregion

#pragma region "Merged Options"

	void ConfigParser::merge_options(ConfigOptions& Options) {
		if (Options.options.find_first_of('n') != std::string::npos) sections["taskmasterd"]["nodaemon"]			= Options.nodaemon;
		if (Options.options.find_first_of('s') != std::string::npos) sections["taskmasterd"]["silent"]				= Options.silent;
		if (Options.options.find_first_of('u') != std::string::npos) sections["taskmasterd"]["user"]				= Options.user;
		if (Options.options.find_first_of('m') != std::string::npos) sections["taskmasterd"]["umask"]				= Options.umask;
		if (Options.options.find_first_of('d') != std::string::npos) sections["taskmasterd"]["directory"]			= Options.directory;
		if (Options.options.find_first_of('l') != std::string::npos) sections["taskmasterd"]["logfile"]				= Options.logfile;
		if (Options.options.find_first_of('y') != std::string::npos) sections["taskmasterd"]["logfile_maxbytes"]	= Options.logfile_maxbytes;
		if (Options.options.find_first_of('z') != std::string::npos) sections["taskmasterd"]["logfile_backups"]		= Options.logfile_backups;
		if (Options.options.find_first_of('e') != std::string::npos) sections["taskmasterd"]["loglevel"]			= Options.loglevel;
		if (Options.options.find_first_of('j') != std::string::npos) sections["taskmasterd"]["pidfile"]				= Options.pidfile;
		if (Options.options.find_first_of('i') != std::string::npos) sections["taskmasterd"]["identifier"]			= Options.identifier;
		if (Options.options.find_first_of('q') != std::string::npos) sections["taskmasterd"]["childlogdir"]			= Options.childlogdir;
		if (Options.options.find_first_of('t') != std::string::npos) sections["taskmasterd"]["strip_ansi"]			= Options.strip_ansi;
		if (Options.options.find_first_of('k') != std::string::npos) sections["taskmasterd"]["nocleanup"]			= Options.nocleanup;
		if (Options.options.find_first_of('a') != std::string::npos) sections["taskmasterd"]["minfds"]				= Options.minfds;
		if (Options.options.find_first_of('p') != std::string::npos) sections["taskmasterd"]["minprocs"]			= Options.minprocs;
	}

#pragma endregion


#pragma region "Print"

	void ConfigParser::print() const {
		for (const auto& section : sections) {
			std::cout << "[" << section.first << "]" << std::endl;
			for (const auto& kv : section.second) {
				std::cout << kv.first << " = " << kv.second << std::endl;
			}
			std::cout << std::endl;
		}
	}

#pragma endregion
