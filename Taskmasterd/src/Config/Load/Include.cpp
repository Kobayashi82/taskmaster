/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Include.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 11:36:32 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/04 12:10:57 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Utils/Utils.hpp"
	#include "Config/Config.hpp"

	#include <cstring>															// strerror()
	#include <fstream>															// std::ifstream
	#include <filesystem>														// std::filesystem::path()

#pragma endregion

#pragma region "Include"

	#pragma region "Load File"

		int ConfigParser::include_load_file(const std::string& filePath) {
			std::string configFile = filePath;

			std::ifstream file(configFile);
			if (!file.is_open()) return (1);

			std::string	line;
			int			lineNumber = 0;
			bool		invalidSection = false;

			while (std::getline(file, line)) { lineNumber++; order += 2;
				if ((line = Utils::trim(Utils::remove_comments(line))).empty()) continue;

				if (is_section(line)) {
					std::string section = section_extract(line);
					if (!section.empty() && section.substr(0, 8) != "program:" && section.substr(0, 6) != "group:") {
						if (section_type(section).empty())	Utils::error_add(configFile, "[" + section + "] unkown section", WARNING, lineNumber, order);
						else								Utils::error_add(configFile, "[" + section + "] invalid section", ERROR, lineNumber, order);
						invalidSection = true; continue;
					}
					invalidSection = section_parse(line, lineNumber, configFile);
				}
				else if (!invalidSection) key_parse(line, lineNumber, configFile);
			}

			return (0);
		}

	#pragma endregion

	#pragma region "Parse Files"

		std::vector<std::string> ConfigParser::include_parse_files(const std::string& fileString, std::string& configFile) {
			std::vector<std::string>	files;
			std::string					current;
			char						quoteChar = 0;
			bool						quotedToken = false;
			bool						escaped = false;

			auto pushToken = [&](bool wasQuoted) {
				if (!current.empty()) {
					std::string token = wasQuoted ? current : Utils::trim(current);
					if (!token.empty()) files.push_back(token);
					current.clear();
				}
			};

			for (char c : fileString) {
				if (escaped)									{ escaped = false;			current.push_back(c);		continue; }
				if (c == '\\')									{ escaped = true;										continue; }
				if (!quoteChar && (c == '"' || c == '\''))		{ quoteChar = c;			quotedToken = true; 		continue; }
				if (quoteChar && c == quoteChar)				{ quoteChar = 0;										continue; }
				if (!quoteChar && c == '\n')					{ pushToken(quotedToken);	quotedToken = false;		continue; }
				current.push_back(c);
			}
			pushToken(quotedToken);

			for (auto& file : files) {
				std::string fullpath = Utils::expand_path(file, std::filesystem::path(configFile).parent_path());
				if (!fullpath.empty()) file = fullpath;
			}

			return (Utils::globbing_expand(files));
		}

	#pragma endregion

	#pragma region "Process"

		void ConfigParser::include_process(std::string& configFile) {
			std::vector<std::string> files = include_parse_files(get_value("include", "files"), configFile);
			currentSection = "";

			for (const auto& file : files) {
				std::string fullpath = Utils::expand_path(file, std::filesystem::path(configFile).parent_path());
				if (fullpath.empty()) fullpath = file;
				if (include_load_file(fullpath)) {
					Utils::error_add(fullpath, "cannot open config file - " + std::string(strerror(errno)), ERROR, 0, order);
					order += 2;
				}
			}
		}

	#pragma endregion

#pragma endregion
