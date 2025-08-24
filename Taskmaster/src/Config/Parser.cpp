/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/24 20:04:14 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/24 22:10:17 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Parser.hpp"
	#include "Utils/Utils.hpp"

	#include <fstream>
	#include <stdexcept>
	#include <algorithm>
	#include <iostream>
	#include <sstream>

#pragma endregion

#pragma region "Constructors"

	Parser::Parser() {
		initialize();
		default_values();
	}

#pragma endregion

#pragma region "Initialize"

	#pragma region "Initialize"

		void Parser::initialize() {
			validSections = {
				"taskmasterd",
				"program:",				// program:name
				"group:",				// group:name
				"unix_http_server",
				"inet_http_server",
				"rpcinterface:"
			};
			validKeys = {
				{"taskmasterd", {
					"nodaemon", "silent", "user", "umask", "directory", "logfile", "logfile_maxbytes", "logfile_backups", "loglevel",
					"pidfile", "identifier", "childlogdir", "strip_ansi", "nocleanup", "minfds", "minprocs", "environment"
				}},
				{"program:", {
					"command", "process_name", "numprocs", "directory", "umask", "priority",
					"autostart", "autorestart", "startsecs", "startretries", "exitcodes",
					"stopsignal", "stopwaitsecs", "stopasgroup", "killasgroup", "user",
					"redirect_stderr", "stdout_logfile", "stderr_logfile", "environment"
				}},
				{"unix_http_server", {
					"file", "chmod", "chown", "username", "password"
				}},
				{"inet_http_server", {
					"port", "username", "password"
				}},
				{"group:", {
					"programs", "priority"
				}},
				{"rpcinterface:", {
					"supervisor.rpcinterface_factory"
				}}
			};
		}

	#pragma endregion

	#pragma region "Default Values"

		void Parser::default_values() {
			std::string	logfile = expand_path("taskmasterd.log");	// $CWD/taskmasterd.log
			std::string	pidfile = expand_path("taskmasterd.pid");	// $CWD/taskmasterd.pid
			std::string	childlogdir = temp_path();

			defaultValues = {
				{"taskmasterd", {
					{"nodaemon", "false"},
					{"silent", "false"},
					{"user", "do not switch"},
					{"umask", "022"},
					{"directory", "do not change"},
					{"logfile", logfile},
					{"logfile_maxbytes", "52428800"},	// 50 MB
					{"logfile_backups", "10"},
					{"loglevel", "1"},
					{"pidfile", pidfile},
					{"identifier", "supervisor"},
					{"childlogdir", childlogdir},
					{"strip_ansi", "false"},
					{"nocleanup", "false"},
					{"minfds", "1024"},
					{"minprocs", "200"},
					{"environment", ""}
				}},
				{"program:", {
					{"numprocs", "1"},
					{"priority", "999"},
					{"autostart", "true"},
					{"autorestart", "unexpected"},
					{"startsecs", "1"},
					{"startretries", "3"},
					{"exitcodes", "0,2"},
					{"stopsignal", "TERM"},
					{"stopwaitsecs", "10"},
					{"stopasgroup", "false"},
					{"killasgroup", "false"},
					{"user", ""},
					{"directory", ""},
					{"umask", "022"},
					{"redirect_stderr", "false"},
					{"stdout_logfile", "AUTO"},
					{"stderr_logfile", "AUTO"},
					{"environment", ""}
				}},
				{"unix_http_server", {
					{"chmod", "0700"},
					{"chown", ""},
					{"username", ""},
					{"password", ""}
				}},
				{"inet_http_server", {
					{"username", ""},
					{"password", ""}
				}},
				{"group:", {
					{"priority", "999"}
				}}
			};
		}

	#pragma endregion

#pragma endregion

#pragma region "Utils"

	#pragma region "Trim"

		std::string Parser::trim(const std::string& str) const {
			size_t first = str.find_first_not_of(" \t\r\n");
			if (first == std::string::npos) return ("");

			size_t last = str.find_last_not_of(" \t\r\n");
			return (str.substr(first, (last - first + 1)));
		}

	#pragma endregion

	#pragma region "To Lower"

		std::string Parser::toLower(const std::string& str) const {
			std::string result = str;
			std::transform(result.begin(), result.end(), result.begin(), ::tolower);

			return (result);
		}

	#pragma endregion

#pragma endregion

#pragma region "Parse"

	#pragma region "Key=Value"

		#pragma region "Is Valid"

			bool Parser::isValidKey(const std::string& section, const std::string& key) const {
				std::string sectionType = SectionType(section);
				if (sectionType.empty()) return (false);

				auto it = validKeys.find(sectionType);
				if (it == validKeys.end()) return (false);

				return (it->second.count(toLower(key)) > 0);
			}

		#pragma endregion

		#pragma region "Parse"

			void Parser::parseKeyValue(const std::string& line) {
				if (currentSection.empty()) throw std::runtime_error("Key-value pair found outside of section: " + line);
				
				size_t pos = line.find('=');
				if (pos == std::string::npos) throw std::runtime_error("Invalid line format (missing =): " + line);
				
				std::string key = trim(line.substr(0, pos));
				std::string value = trim(line.substr(pos + 1));
				
				if (key.empty()) throw std::runtime_error("Empty key in line: " + line);
				if (!isValidKey(currentSection, key)) throw std::runtime_error("Invalid key '" + key + "' in section [" + currentSection + "]");
				
				std::string normalizedKey = toLower(key);
				sections[currentSection][normalizedKey] = value;
			}

		#pragma endregion

	#pragma endregion

	#pragma region "Section"

		#pragma region "Type"

			std::string Parser::SectionType(const std::string& section) const {
				for (const auto& validSection : validSections) {
					if (validSection.back() == ':') {
						if (section.substr(0, validSection.length()) == validSection)	return (validSection);
					} else if (section == validSection)									return (validSection);
				}

				return ("");
			}

		#pragma endregion

		#pragma region "Is Valid"

			bool Parser::isValidSection(const std::string& section) const {
				if (validSections.count(section)) return (true);

				for (const auto& validSection : validSections) {
					if (validSection.back() == ':' &&
						section.substr(0, validSection.length()) == validSection &&
						section.length() > validSection.length()) {
						return (true);
					}
				}

				return (false);
			}

		#pragma endregion

		#pragma region "Extract"

			std::string Parser::extractSection(const std::string& line) const {
				std::string trimmed = trim(line);
				return (trimmed.substr(1, trimmed.length() - 2));
			}

		#pragma endregion

		#pragma region "Parse"

			void Parser::parseSection(const std::string& line) {
				std::string section = extractSection(line);

				if (!isValidSection(section)) throw std::runtime_error("Invalid section: [" + section + "]");
				currentSection = section;

				if (sections.find(currentSection) == sections.end()) {
					sections[currentSection] = std::map<std::string, std::string>();
				}
			}

		#pragma endregion

	#pragma endregion

	#pragma region "Line"

		#pragma region "Is Section"

			bool Parser::isSection(const std::string& line) const {
				std::string trimmed = trim(line);
				return trimmed.size() >= 2 && trimmed[0] == '[' && trimmed.back() == ']';
			}

		#pragma endregion

		#pragma region "Is Comment"

			bool Parser::isComment(const std::string& line) const {
				std::string trimmed = trim(line);
				return (trimmed.empty() || trimmed[0] == '#' || trimmed[0] == ';');
			}

		#pragma endregion

		#pragma region "Parse"

			void Parser::parseLine(const std::string& line) {
				if (isComment(line))	return;
				if (isSection(line))	parseSection(line);
				else					parseKeyValue(line);
			}

		#pragma endregion

	#pragma endregion

	#pragma region "File"

		int Parser::parseFile(const std::string& filePath) {
			std::ifstream file(filePath);
			if (!file.is_open()) throw std::runtime_error("Cannot open config file: " + filePath);

			clear();

			std::string	line;
			int			lineNumber = 0;

			try {
				while (std::getline(file, line)) {
					lineNumber++;
					parseLine(line);
				}
			} catch (const std::exception& e) { throw std::runtime_error("Error at line " + std::to_string(lineNumber) + ": " + e.what()); }

			return (0);
		}

	#pragma endregion

#pragma endregion

#pragma region "Getters"

	#pragma region "Has Section"

		bool Parser::hasSection(const std::string& section) const {
			return (sections.find(section) != sections.end());
		}

	#pragma endregion

	#pragma region "Value"

		std::string Parser::getValue(const std::string& section, const std::string& key, const std::string& defaultValue) const {
			auto sectionIt = sections.find(section);
			std::string normalizedKey = toLower(key);
			
			// Buscar en configuración parseada
			if (sectionIt != sections.end()) {
				auto keyIt = sectionIt->second.find(normalizedKey);
				if (keyIt != sectionIt->second.end()) return (keyIt->second);
			}
			
			// Buscar en defaults
			std::string sectionType = SectionType(section);
			if (!sectionType.empty()) {
				auto defaultSectionIt = defaultValues.find(sectionType);
				if (defaultSectionIt != defaultValues.end()) {
					auto defaultKeyIt = defaultSectionIt->second.find(normalizedKey);
					if (defaultKeyIt != defaultSectionIt->second.end()) return (defaultKeyIt->second);
				}
			}
			
			// Fallback a defaultValue
			return (defaultValue);
		}

	#pragma endregion

	#pragma region "Section"

		std::map<std::string, std::string> Parser::getSection(const std::string& section) const {
			auto it = sections.find(section);
			if (it != sections.end()) return (it->second);

			return (std::map<std::string, std::string>());
		}

	#pragma endregion

	#pragma region "Section with Defaults"

		std::map<std::string, std::string> Parser::getSectionWithDefaults(const std::string& section) const {
			std::map<std::string, std::string> result;

			// Empezar con los defaults
			std::string sectionType = SectionType(section);
			if (!sectionType.empty()) {
				auto defaultSectionIt = defaultValues.find(sectionType);
				if (defaultSectionIt != defaultValues.end()) result = defaultSectionIt->second;
			}

			// Sobrescribir con valores del archivo
			auto sectionIt = sections.find(section);
			if (sectionIt != sections.end()) {
				for (const auto& kv : sectionIt->second) result[kv.first] = kv.second;
			}

			return (result);
		}

	#pragma endregion

	#pragma region "Program"

		std::vector<std::string> Parser::getProgramSections() const {
			std::vector<std::string> programs;
			for (const auto& section : sections) {
				if (section.first.substr(0, 8) == "program:") programs.push_back(section.first);
			}

			return (programs);
		}

	#pragma endregion

	#pragma region "Group"

		std::vector<std::string> Parser::getGroupSections() const {
			std::vector<std::string> groups;
			for (const auto& section : sections) {
				if (section.first.substr(0, 6) == "group:") groups.push_back(section.first);
			}

			return (groups);
		}

	#pragma endregion

#pragma endregion

#pragma region "Debug"

	#pragma region "Clear"

	void Parser::clear() {
		currentSection.clear();
		sections.clear();
	}

#pragma endregion

	#pragma region "Print"

	void Parser::printConfig() const {
		for (const auto& section : sections) {
			std::cout << "[" << section.first << "]" << std::endl;
			for (const auto& kv : section.second) {
				std::cout << kv.first << " = " << kv.second << std::endl;
			}
			std::cout << std::endl;
		}
	}

#pragma endregion

#pragma endregion
