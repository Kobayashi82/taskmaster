/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Include.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 11:36:32 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/30 12:47:30 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Parser.hpp"

	#include <fstream>															// std::ifstream
	#include <filesystem>														// std::filesystem::path()

#pragma endregion

#pragma region "Include"

	#pragma region "File"

		void ConfigParser::include_parse(const std::string& filePath) {
			std::ifstream file(filePath);
			if (!file.is_open()) throw std::runtime_error("[" + filePath + "]\nError:\t\t\tCannot open config file\n");

			std::string	configDir = std::filesystem::path(filePath).parent_path();
			std::string	line, errors;
			int			lineNumber = 0;
			bool		invalid_section = false;

			environment_add(environment_config, "HERE", std::filesystem::path(filePath).parent_path());

			while (std::getline(file, line)) { lineNumber++;
				line = trim(key_remove_comments(line));
				if (line.empty()) continue;

				try {
					if (is_section(line)) {
						std::string section = section_extract(line);
						if (!section.empty() && section.substr(0, 8) != "program:" && section.substr(0, 6) != "group:")
							throw std::runtime_error("[" + section + "] invalid section");
						section_parse(line);
						invalid_section = false;
					}
					else if (invalid_section)	continue;
					else						key_parse(line);
				}
				catch (const std::exception& e) {
					if (std::string(e.what()).find("Ignore section") != std::string::npos) {
						errors += "Error at line " + std::to_string(lineNumber) + ":\t[" + currentSection + "] invalid section\n";
						invalid_section = true; continue;
					}
					if	(line == "[include]" && std::string(e.what()).find("duplicate section") != std::string::npos) {
						errors += "Error at line " + std::to_string(lineNumber) + ":\t[include] invalid section\n";
						invalid_section = true; continue;
					}
					if (std::string(e.what()).find("invalid section") != std::string::npos)		invalid_section = true;
					if (std::string(e.what()).find("duplicate section") != std::string::npos)	invalid_section = true;
					errors += "Error at line " + std::to_string(lineNumber) + ":\t" + e.what() + "\n";
				}
			}
			in_include = false;
			invalid_section = true;

			if (!errors.empty()) throw std::runtime_error("[" + filePath + "]\n" + errors);
		}

	#pragma endregion

	#pragma region "Process"

		void ConfigParser::include_process() {
			std::vector<std::string> files = parse_files(get_value("include", "files"));
			std::string errors;

			currentSection = "";

			for (const auto& file : files) {
				try {
					std::string fullpath = expand_path(file, configPath.parent_path());
					if (fullpath.empty()) fullpath = file;
					include_parse(fullpath);
				}
				catch (const std::exception& e) { errors += ((errors.empty()) ? "" : "\n") + std::string(e.what()); }
			}

			if (!errors.empty()) throw std::runtime_error(errors);
		}

	#pragma endregion

#pragma endregion
