/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Include.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 11:36:32 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/27 14:16:04 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Parser.hpp"

	#include <fstream>															// std::ifstream
	#include <filesystem>														// std::filesystem::path()

#pragma endregion

#pragma region "Include"

	#pragma region "File"

		void ConfigParser::parse_include_file(const std::string& filePath) {
			std::ifstream file(filePath);
			if (!file.is_open()) throw std::runtime_error("[" + filePath + "]\nError:\t\t\tCannot open config file\n");

			std::string configDir = std::filesystem::path(filePath).parent_path();
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
					if (is_section(line)) {
						std::string section = extract_section(line);
						if (!section.empty() && section.substr(0, 8) != "program:" && section.substr(0, 6) != "group:")
							throw std::runtime_error("Invalid section:\t[" + section + "]");
						parse_section(line);
						invalid_section = false;
					}
					else if (invalid_section)	continue;
					else						parse_key(line);
				}
				catch (const std::exception& e) {
					if (std::string(e.what()).substr(0, 14) == "Ignore section") {
						errors += "Error at line " + std::to_string(lineNumber) + ":\tInvalid section:\t[" + currentSection + "]\n";
						invalid_section = true; continue;
					}
					if	(line == "[include]" && std::string(e.what()).substr(0, 17) == "Duplicate section") {
						errors += "Error at line " + std::to_string(lineNumber) + ":\tInvalid section:\t[include]\n";
						invalid_section = true; continue;
					}
					if (std::string(e.what()).substr(0, 15) == "Invalid section")	invalid_section = true;
					if (std::string(e.what()).substr(0, 17) == "Duplicate section")	invalid_section = true;
					errors += "Error at line " + std::to_string(lineNumber) + ":\t" + e.what() + "\n";
				}
			}
			in_include = false;
			invalid_section = true;

			if (!errors.empty()) throw std::runtime_error("[" + filePath + "]\n" + errors);
		}

	#pragma endregion

	#pragma region "Process"

		void ConfigParser::process_includes() {
			std::vector<std::string> files = parse_files(get_value("include", "files"));
			std::string errors;

			for (const auto& file : files) {
				try {
					std::string fullpath = expand_path(file, configPath.parent_path());
					if (fullpath.empty()) fullpath = file;
					parse_include_file(fullpath);
				}
				catch (const std::exception& e) { errors += ((errors.empty()) ? "" : "\n") + std::string(e.what()); }
			}

			if (!errors.empty()) throw std::runtime_error(errors);
		}

	#pragma endregion

#pragma endregion
