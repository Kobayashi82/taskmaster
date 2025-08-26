/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/24 20:04:14 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/26 12:19:32 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Utils/Utils.hpp"
	#include "Config/Options.hpp"
	#include "Config/Parser.hpp"

	#include <fstream>															// std::ifstream
	#include <sstream>															// std::istringstream
	#include <algorithm>														// std::transform()
	#include <filesystem>														// std::filesystem::path(), std::filesystem::canonical()
	#include <unistd.h>															// access()
	#include <pwd.h>															// struct passwd, getpwnam()

#pragma endregion

#pragma region "Variables"

	ConfigParser Parser;

#pragma endregion

#pragma region "Constructors"

	ConfigParser::ConfigParser() {
		initialize();
		default_values();
	}

#pragma endregion

#pragma region "Initialize"

	#pragma region "Initialize"

		void ConfigParser::initialize() {
			validSections = { "taskmasterd", "program:", "group:", "unix_http_server", "inet_http_server", "include", "taskmasterctl" };
			validKeys = {
				{"taskmasterd", {
					"nodaemon", "silent", "user", "umask", "directory", "logfile", "logfile_maxbytes", "logfile_backups", "loglevel",
					"pidfile", "identifier", "childlogdir", "strip_ansi", "nocleanup", "minfds", "minprocs", "environment"
				}},
				{"program:", {
					"command", "process_name", "numprocs", "directory", "umask", "priority", "autostart", "autorestart",
					"startsecs", "startretries", "exitcodes", "stopsignal", "stopwaitsecs", "stopasgroup", "killasgroup", "user",
					"redirect_stderr", "stdout_logfile", "stderr_logfile", "environment"
				}},
				{"unix_http_server",	{ "file", "chmod", "chown", "username", "password" }},
				{"inet_http_server",	{ "port", "username", "password" }},
				{"group:",				{ "programs", "priority" }},
				{"include",				{ "files" }}
			};
		}

	#pragma endregion

	#pragma region "Default Values"

		void ConfigParser::default_values() {
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
					{"user", "do not switch"},
					{"directory", "do not change"},
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

		std::string ConfigParser::trim(const std::string& str) const {
			size_t first = str.find_first_not_of(" \t\r\n");
			if (first == std::string::npos) return ("");

			size_t last = str.find_last_not_of(" \t\r\n");
			return (str.substr(first, (last - first + 1)));
		}

	#pragma endregion

	#pragma region "To Lower"

		std::string ConfigParser::toLower(const std::string& str) const {
			std::string result = str;
			std::transform(result.begin(), result.end(), result.begin(), ::tolower);

			return (result);
		}

	#pragma endregion

#pragma endregion

#pragma region "Parse"

	#pragma region "Key=Value"

		#pragma region "Is Valid"

			bool ConfigParser::isValidKey(const std::string& section, const std::string& key) const {
				std::string sectionType = SectionType(section);
				if (sectionType.empty()) return (false);

				auto it = validKeys.find(sectionType);
				if (it == validKeys.end()) return (false);

				return (it->second.count(toLower(key)) > 0);
			}

		#pragma endregion

		#pragma region "Parse"

			void ConfigParser::parseKeyValue(const std::string& line) {
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

			std::string ConfigParser::SectionType(const std::string& section) const {
				for (const auto& validSection : validSections) {
					if (validSection.back() == ':') {
						if (section.substr(0, validSection.length()) == validSection)	return (validSection);
					} else if (section == validSection)									return (validSection);
				}

				return ("");
			}

		#pragma endregion

		#pragma region "Is Valid"

			bool ConfigParser::isValidSection(const std::string& section) const {
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

			std::string ConfigParser::extractSection(const std::string& line) const {
				std::string trimmed = trim(line);
				return (trimmed.substr(1, trimmed.length() - 2));
			}

		#pragma endregion

		#pragma region "Parse"

			void ConfigParser::parseSection(const std::string& line) {
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

			bool ConfigParser::isSection(const std::string& line) const {
				std::string trimmed = trim(line);
				return trimmed.size() >= 2 && trimmed[0] == '[' && trimmed.back() == ']';
			}

		#pragma endregion

		#pragma region "Is Comment"

			bool ConfigParser::isComment(const std::string& line) const {
				std::string trimmed = trim(line);
				return (trimmed.empty() || trimmed[0] == '#' || trimmed[0] == ';');
			}

		#pragma endregion

		#pragma region "Parse"

			void ConfigParser::parseLine(const std::string& line) {
				if (isComment(line))	return;
				if (isSection(line))	parseSection(line);
				else					parseKeyValue(line);
			}

		#pragma endregion

	#pragma endregion

	#pragma region "File"

		void ConfigParser::parseFile(const std::string& filePath) {
			std::ifstream file(filePath);
			if (!file.is_open()) throw std::runtime_error("Cannot open config file: " + filePath);

			clear();

			std::string	line;
			std::string	errors;
			int			lineNumber = 0;

			while (std::getline(file, line)) {
				lineNumber++;
				try { parseLine(line); }
				catch (const std::exception& e) { errors += "Error at line " + std::to_string(lineNumber) + ": " + e.what() + "\n"; }
			}

			if (!errors.empty()) throw std::runtime_error(errors);
		}

	#pragma endregion

#pragma endregion

#pragma region "Validation"

	#pragma region "Helpers"

		#pragma region "Bool"

			bool ConfigParser::isValidBool(const std::string& value) const {
				std::string lower = toLower(value);
				return (lower == "true" || lower == "false" || lower == "1" || lower == "0" || lower == "yes" || lower == "no");
			}

		#pragma endregion

		#pragma region "Number"

			bool ConfigParser::isValidNumber(const std::string& value, long min, long max) const {
				if (value.empty()) return (false);

				char	*end;
				long	num = std::strtol(value.c_str(), &end, 10);

				return (*end == '\0' && num >= min && num <= max);
			}

		#pragma endregion

		#pragma region "Path"

			bool ConfigParser::isValidPath(const std::string& value, bool is_directory) const {
				if (value.empty()) return (false);
				if (value == "do not change") return (true);

				std::string fullPath = expand_path(value);
				if (fullPath.empty()) return (false);

				std::filesystem::path p(fullPath);

				if (is_directory) return (std::filesystem::exists(p) && std::filesystem::is_directory(p) && !access(p.c_str(), W_OK | X_OK));

				std::filesystem::path dir = p.parent_path();
				if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir)) return (false);
				if (access(dir.c_str(), W_OK)) return (false);

				if (std::filesystem::exists(p)) {
					if (std::filesystem::is_directory(p)) return (false);
					if (access(p.c_str(), W_OK)) return (false);
				}

				return (true);
			}

		#pragma endregion

		#pragma region "Signal"

			bool ConfigParser::isValidSignal(const std::string& value) const {
				std::string upper = value;
				std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

				std::set<std::string> validSignals = { "TERM", "HUP", "INT", "QUIT", "KILL", "USR1", "USR2" };

				return (validSignals.count(upper) > 0);
			}

		#pragma endregion

		#pragma region "Exit Code"

			bool ConfigParser::isValidExitCodes(const std::string& value) const {
				if (value.empty()) return (true);

				std::istringstream ss(value);
				std::string code;

				while (std::getline(ss, code, ',')) {
					code = trim(code);
					if (!isValidNumber(code, 0, 255)) return (false);
				}

				return (true);
			}

		#pragma endregion

		#pragma region "Loglevel"

			bool ConfigParser::isValidLogLevel(const std::string& value) const {
				std::string lower = toLower(value);

				if (lower == "debug" || lower == "info" || lower == "warn" || lower == "warning" || lower == "error" || lower == "critical") return (true);
				return (isValidNumber(value, 0, 4));	// 0=DEBUG, 1=INFO, 2=WARN, 3=ERROR, 4=CRITICAL
			}

		#pragma endregion

		#pragma region "Autorestart"

			bool ConfigParser::isValidAutorestart(const std::string& value) const {
				std::string lower = toLower(value);

				return (lower == "false" || lower == "unexpected" || lower == "true" || lower == "yes" || lower == "no" || lower == "0" || lower == "1");
			}

		#pragma endregion

		#pragma region "Umask"

			bool ConfigParser::isValidUmask(const std::string& value) const {
				if (value.length() != 3 && value.length() != 4) return (false);

				int start = 0;
				if (value.length() == 4) {
					if (value[0] != '0') return (false);
					start = 1;
				}

				for (size_t i = start; i < value.length(); ++i) {
					if (value[i] < '0' || value[i] > '7') return (false);
				}

				return (true);
			}

		#pragma endregion
	
		#pragma region "User"

			bool ConfigParser::isValidUser(const std::string& value) const {
				if (value.empty()) return (false);
				if (value == "do not switch") return (true);

				struct passwd *pw = getpwnam(value.c_str());
				if (!pw) return (false);
				if (pw->pw_shell && std::string(pw->pw_shell).find("nologin") != std::string::npos) return (false);

				return (true);
			}

		#pragma endregion

	#pragma endregion

	#pragma region "Validators"

		#pragma region "Taskmasterd"

			void ConfigParser::validateTaskmasterdSection(const std::string& section, std::string& errors) const {
				auto config = getSectionWithDefaults(section);

				// Boolean values
				if (!isValidBool(config["nodaemon"]))
					errors += "[" + section + "] nodaemon must be true or false\n";

				if (!isValidBool(config["silent"]))
					errors += "[" + section + "] silent must be true or false\n";

				if (!isValidBool(config["strip_ansi"]))
					errors += "[" + section + "] strip_ansi must be true or false\n";

				if (!isValidBool(config["nocleanup"]))
					errors += "[" + section + "] nocleanup must be true or false\n";

				// Numeric values
				long bytes = parseSize(config["logfile_maxbytes"]);
				if (bytes == -1 || !isValidNumber(std::to_string(bytes), 0, INT_MAX))
					errors += "[" + section + "] logfile_maxbytes must be between 0 bytes and 2 GB\n";

				if (!isValidNumber(config["logfile_backups"], 0, 1000))
					errors += "[" + section + "] logfile_backups must be between 0 and 1000\n";

				if (!isValidLogLevel(config["loglevel"]))
					errors += "[" + section + "] loglevel must be one of: DEBUG, INFO, WARN, ERROR, CRITICAL\n";

				if (!isValidNumber(config["minfds"], 1, 65535))
					errors += "[" + section + "] minfds must be between 1 and 65535\n";
				if (check_fd_limit(static_cast<uint16_t>(std::stoul(config["minfds"]))))
					errors += "[" + section + "] minfds limit could not be applied (system limit too low or insufficient permissions)\n";

				if (!isValidNumber(config["minprocs"], 1, 10000))
					errors += "[" + section + "] minprocs must be between 1 and 10000\n";
				if (check_process_limit(static_cast<uint16_t>(std::stoul(config["minprocs"]))))
					errors += "[" + section + "] minprocs limit could not be applied (system limit too low or insufficient permissions)\n";

				// Path validation
				if (!isValidPath(config["directory"], true))
					errors += "[" + section + "] directory path is invalid\n";

				if (!isValidPath(config["logfile"], false))
					errors += "[" + section + "] logfile path is invalid\n";

				if (!isValidPath(config["pidfile"], false))
					errors += "[" + section + "] pidfile path is invalid\n";

				if (!isValidPath(config["childlogdir"], true))
					errors += "[" + section + "] childlogdir path is invalid\n";

				// Umask validation
				if (!isValidUmask(config["umask"]))
					errors += "[" + section + "] umask must be in octal format (e.g., 022)\n";
			}

		#pragma endregion

		#pragma region "Program"

			void ConfigParser::validateProgramSection(const std::string& section, std::string& errors) const {
				auto config = getSectionWithDefaults(section);

				// Required field
				// Verified command exists
				if (config.find("command") == config.end() || config["command"].empty())
					errors += "[" + section + "] command is required\n";

				// Boolean values
				if (!isValidBool(config["autostart"]))
					errors += "[" + section + "] autostart must be true or false\n";

				if (!isValidBool(config["stopasgroup"]))
					errors += "[" + section + "] stopasgroup must be true or false\n";

				if (!isValidBool(config["killasgroup"]))
					errors += "[" + section + "] killasgroup must be true or false\n";

				if (!isValidBool(config["redirect_stderr"]))
					errors += "[" + section + "] redirect_stderr must be true or false\n";

				// Numeric values
				if (!isValidNumber(config["numprocs"], 1, 1000))
					errors += "[" + section + "] numprocs must be between 1 and 1000\n";

				if (!isValidNumber(config["priority"], 0, 999))
					errors += "[" + section + "] priority must be between 0 and 999\n";

				if (!isValidNumber(config["startsecs"], 0, 3600))
					errors += "[" + section + "] startsecs must be between 0 and 3600\n";

				if (!isValidNumber(config["startretries"], 0, 100))
					errors += "[" + section + "] startretries must be between 0 and 100\n";

				if (!isValidNumber(config["stopwaitsecs"], 1, 3600))
					errors += "[" + section + "] stopwaitsecs must be between 1 and 3600\n";

				// Special validations
				if (!isValidAutorestart(config["autorestart"]))
					errors += "[" + section + "] autorestart must be false, unexpected or true\n";

				if (!isValidExitCodes(config["exitcodes"]))
					errors += "[" + section + "] exitcodes must be comma-separated numbers between 0 and 255\n";

				if (!isValidSignal(config["stopsignal"]))
					errors += "[" + section + "] stopsignal must be a valid signal (TERM, HUP, INT, QUIT, KILL, USR1, USR2)\n";

				// Path validation
				if (!isValidPath(config["directory"], true))
					errors += "[" + section + "] directory path is invalid\n";

				// std_out, std_err: NONE, AUTO
				// AUTO = childlogdir

				// Umask validation
				if (!config["umask"].empty() && !isValidUmask(config["umask"]))
					errors += "[" + section + "] umask must be in octal format (e.g., 022)\n";
			}

		#pragma endregion

		#pragma region "Group"

			void ConfigParser::validateGroupSection(const std::string& section, std::string& errors) const {
				auto config = getSectionWithDefaults(section);

				// Required field
				if (config.find("programs") == config.end() || config["programs"].empty())
					errors += "[" + section + "] programs is required\n";

				// Priority validation
				if (!isValidNumber(config["priority"], 0, 999))
					errors += "[" + section + "] priority must be between 0 and 999\n";

				// Validate that referenced programs exist
				std::string programs = config["programs"];
				std::istringstream ss(programs);
				std::string program;

				while (std::getline(ss, program, ',')) {
					program = trim(program);
					std::string programSection = "program:" + program;
					if (!hasSection(programSection))
						errors += "[" + section + "] references non-existent program: " + program + "\n";
				}
			}

		#pragma endregion

		#pragma region "Unix Server"

			void ConfigParser::validateUnixHttpServerSection(const std::string& section, std::string& errors) const {
				auto config = getSectionWithDefaults(section);

				// Required field
				// Can be created
				if (config.find("file") == config.end() || config["file"].empty())
					errors += "[" + section + "] file is required\n";

				// Permission validation
				if (!config["chmod"].empty() && !isValidUmask(config["chmod"]))
					errors += "[" + section + "] chmod must be in octal format (e.g., 700)\n";
			}

		#pragma endregion

		#pragma region "Inet Server"

			void ConfigParser::validateInetHttpServerSection(const std::string& section, std::string& errors) const {
				auto config = getSectionWithDefaults(section);

				// Required field
				if (config.find("port") == config.end() || config["port"].empty())
					errors += "[" + section + "] port is required\n";

				// Port validation
				if (!isValidNumber(config["port"], 1, 65535))
					errors += "[" + section + "] port must be between 1 and 65535\n";
			}

		#pragma endregion

	#pragma endregion

	#pragma region "Validate"

		void ConfigParser::validate() const {
			std::string errors;

			for (const auto& section : sections) {
				std::string sectionName = section.first;
				std::string sectionType = SectionType(sectionName);

				if		(sectionType == "taskmasterd")			validateTaskmasterdSection(sectionName, errors);
				else if	(sectionType == "program:")				validateProgramSection(sectionName, errors);
				else if	(sectionType == "unix_http_server")		validateUnixHttpServerSection(sectionName, errors);
				else if	(sectionType == "inet_http_server")		validateInetHttpServerSection(sectionName, errors);
				else if	(sectionType == "group:")				validateGroupSection(sectionName, errors);
			}

			if (!errors.empty()) throw std::runtime_error(errors);
		}

	#pragma endregion

#pragma endregion

#pragma region "Getters"

	#pragma region "Has Section"

		bool ConfigParser::hasSection(const std::string& section) const {
			return (sections.find(section) != sections.end());
		}

	#pragma endregion

	#pragma region "Value"

		std::string ConfigParser::getValue(const std::string& section, const std::string& key, const std::string& defaultValue) const {
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

		std::map<std::string, std::string> ConfigParser::getSection(const std::string& section) const {
			auto it = sections.find(section);
			if (it != sections.end()) return (it->second);

			return (std::map<std::string, std::string>());
		}

	#pragma endregion

	#pragma region "Section with Defaults"

		std::map<std::string, std::string> ConfigParser::getSectionWithDefaults(const std::string& section) const {
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

		std::vector<std::string> ConfigParser::getProgramSections() const {
			std::vector<std::string> programs;
			for (const auto& section : sections) {
				if (section.first.substr(0, 8) == "program:") programs.push_back(section.first);
			}

			return (programs);
		}

	#pragma endregion

	#pragma region "Group"

		std::vector<std::string> ConfigParser::getGroupSections() const {
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

	void ConfigParser::clear() {
		currentSection.clear();
		sections.clear();
	}

#pragma endregion

	#pragma region "Print"

	#include <iostream>

	void ConfigParser::printConfig() const {
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

#pragma region "Add OPT Args"

	void ConfigParser::add_opt_args(ConfigOptions& Options) {
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
