/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Include.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 11:36:32 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/01 13:43:54 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Parser.hpp"
	#include "Logging/TaskmasterLog.hpp"

	#include <cstring>															// strerror()
	#include <fstream>															// std::ifstream
	#include <filesystem>														// std::filesystem::path()

#pragma endregion

#pragma region "Include"

	#pragma region "File"

		int ConfigParser::include_parse(const std::string& filePath) {
			std::string configFile = filePath;

			std::ifstream file(configFile);
			if (!file.is_open()) return (1);

			std::string	line, configDir = std::filesystem::path(configFile).parent_path();
			int			lineNumber = 0;
			bool		invalidSection = false;

			while (std::getline(file, line)) { lineNumber++; order++;
				line = trim(remove_comments(line));
				if (line.empty()) continue;

				if (is_section(line)) {
					std::string section = section_extract(line);
					if (!section.empty() && section.substr(0, 8) != "program:" && section.substr(0, 6) != "group:") {
						error_add(configFile, "[" + section + "] invalid section", WARNING, lineNumber, order);
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
			bool						inQuotes = false;
			bool						quotedToken = false;
			char						quoteChar = 0;

			auto pushToken = [&](bool wasQuoted) {
				if (!current.empty()) {
					std::string token = wasQuoted ? current : trim(current);
					if (!token.empty()) files.push_back(token);
					current.clear();
				}
			};

			for (size_t i = 0; i < fileString.size(); ++i) {
				char c = fileString[i];

				if		(c == '\\' && i + 1 < fileString.size())			  current.push_back(fileString[++i]);
				else if	((c == '"' || c == '\'') && !inQuotes)				{ inQuotes = quotedToken = true; quoteChar = c; }
				else if	(inQuotes && c == quoteChar)						  inQuotes = false;
				else if	(!inQuotes && (c == ' ' || c == ',' || c == '\t'))	{ pushToken(quotedToken); quotedToken = false; }
				else														  current.push_back(c);
			}

			pushToken(quotedToken);

			for (auto& file : files) {
				std::string fullpath = expand_path(file, std::filesystem::path(configFile).parent_path(), false);
				if (!fullpath.empty()) file = fullpath;
			}

			files = globbing_expand(files);

			return (files);
		}

	#pragma endregion

	#pragma region "Process"

		void ConfigParser::include_process(std::string& configFile, int lineNumber) {
			std::vector<std::string> files = include_parse_files(get_value("include", "files"), configFile);

			currentSection = "";

			for (const auto& file : files) {
				std::string fullpath = expand_path(file, std::filesystem::path(configFile).parent_path());
				if (fullpath.empty()) fullpath = file;
				if (include_parse(fullpath)) error_add(configFile, "cannot open config file in include section: " + fullpath + " - " + strerror(errno), ERROR, lineNumber, order++);
			}
		}

	#pragma endregion

#pragma endregion
