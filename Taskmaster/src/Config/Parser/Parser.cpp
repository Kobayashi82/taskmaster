/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 11:33:13 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/28 15:33:46 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Options.hpp"
	#include "Config/Parser.hpp"

	#include <fstream>															// std::ifstream
	#include <iostream>															// std::cout

#pragma endregion

#pragma region "Variables"

	ConfigParser Parser;

#pragma endregion

#pragma region "Constructors"

	ConfigParser::ConfigParser() {
		section_on_error = true;
		initialize();
		default_values();
	}

#pragma endregion

#pragma region "File"

	void ConfigParser::parse_file(const std::string& filePath) {
		configPath = expand_path(filePath);
		if (configPath.empty()) configPath = filePath;

		std::ifstream file(configPath);
		if (!file.is_open()) throw std::runtime_error("Cannot open config file: " + filePath + "\n");

		environment_initialize();
		currentSection.clear();
		sections.clear();

		std::string line;
		std::string errors;
		bool invalid_section = false;
		int lineNumber = 0;

		while (std::getline(file, line)) {
			lineNumber++;
			size_t pos = line.find_first_of(";#");
			if (pos != std::string::npos) line = line.substr(0, pos);
			line = trim(line);
			if (line.empty()) continue;

			try {
				if (in_include && is_section(line)) {
					std::string section = extract_section(line);
					if (section != "include" && in_include) {
						in_include = false; currentSection = "";
						try { process_includes(); invalid_section = false; }
						catch (const std::exception& e) {
							errors += ((errors.empty()) ? "" : "\n") + std::string(e.what());
							section_on_error = true; invalid_section = false;
						}
					}
				}
				if (is_section(line)) {		parse_section(line); invalid_section = false; }
				else if (invalid_section)	continue;
				else						parse_key(line);
			}
			catch (const std::exception& e) {
				if (std::string(e.what()).substr(0, 14) == "Ignore section")	{ invalid_section = true; continue; }
				if (section_on_error) {
					errors += ((errors.empty()) ? "" : "\n") + std::string("[" + configPath.string() + "]\n");
					section_on_error = false;
				}
				errors += "Error at line " + std::to_string(lineNumber) + ":\t" + e.what() + "\n";
				if (std::string(e.what()).substr(0, 15) == "Invalid section")	invalid_section = true;
				if (std::string(e.what()).substr(0, 17) == "Duplicate section")	invalid_section = true;
			}
		}

		if (in_include) {
			in_include = false; currentSection = "";
			try { process_includes(); }
			catch (const std::exception& e) { errors += ((errors.empty()) ? "" : "\n") + std::string(e.what()); }
		}

		std::map<std::string, std::string> program_env;
		std::map<std::string, std::string> program_src;
		environment_add(program_src, "KEY", "VALUE");
		environment_clone(program_env, program_src);
		environment_add_batch(program_env, get_value("taskmasterd", "environment"));
		environment_print(program_env);

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
