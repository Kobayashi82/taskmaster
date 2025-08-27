/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Validation.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 11:32:25 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/27 12:21:18 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Parser.hpp"

	#include <unistd.h>															// access()
	#include <filesystem>														// std::filesystem::path(), std::filesystem::is_directory(), std::filesystem::exists()
	#include <pwd.h>															// struct passwd, getpwnam()

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
				std::set<std::string> validSignals = { "TERM", "HUP", "INT", "QUIT", "KILL", "USR1", "USR2" };
				return (validSignals.count(toUpper(value)) > 0);
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

			void ConfigParser::validateTaskmasterdSection(const std::string& section, const std::string& key, std::string& value) const {
				// Boolean values
				if (key == "nodaemon" && !isValidBool(value))
					throw std::runtime_error("[" + section + "] nodaemon must be true or false");

				if (key == "silent" && !isValidBool(value))
					throw std::runtime_error("[" + section + "] silent must be true or false");

				if (key == "strip_ansi" && !isValidBool(value))
					throw std::runtime_error("[" + section + "] strip_ansi must be true or false");

				if (key == "nocleanup" && !isValidBool(value))
					throw std::runtime_error("[" + section + "] nocleanup must be true or false");

				// Numeric values
				if (key == "logfile_maxbytes") {
					long bytes = parse_size(value);
					if (bytes == -1 || !isValidNumber(std::to_string(bytes), 0, INT_MAX))
						throw std::runtime_error("[" + section + "] logfile_maxbytes must be between 0 bytes and 2 GB");
				}

				if (key == "logfile_backups" && !isValidNumber(value, 0, 1000))
					throw std::runtime_error("[" + section + "] logfile_backups must be between 0 and 1000");

				if (key == "loglevel" && !isValidLogLevel(value))
					throw std::runtime_error("[" + section + "] loglevel must be one of: DEBUG, INFO, WARN, ERROR, CRITICAL");

				if (key == "minfds") {
					if (!isValidNumber(value, 1, 65535))
						throw std::runtime_error("[" + section + "] minfds must be between 1 and 65535");
					if (check_fd_limit(static_cast<uint16_t>(std::stoul(value))))
						throw std::runtime_error("[" + section + "] minfds limit could not be applied (system limit too low or insufficient permissions)");
				}

				if (key == "minprocs") {
					if (!isValidNumber(value, 1, 10000))
						throw std::runtime_error("[" + section + "] minprocs must be between 1 and 10000");
					if (check_process_limit(static_cast<uint16_t>(std::stoul(value))))
						throw std::runtime_error("[" + section + "] minprocs limit could not be applied (system limit too low or insufficient permissions)");
				}

				// Path validation
				if (key == "directory" && !isValidPath(value, true))
					throw std::runtime_error("[" + section + "] directory path is invalid");

				if (key == "logfile" && !isValidPath(value, false))
					throw std::runtime_error("[" + section + "] logfile path is invalid");

				if (key == "pidfile" && !isValidPath(value, false))
					throw std::runtime_error("[" + section + "] pidfile path is invalid");

				if (key == "childlogdir" && !isValidPath(value, true))
					throw std::runtime_error("[" + section + "] childlogdir path is invalid");

				// Umask validation
				if (key == "umask" && !isValidUmask(value))
					throw std::runtime_error("[" + section + "] umask must be in octal format (e.g., 022)");
			}

		#pragma endregion

		#pragma region "Program"

			void ConfigParser::validateProgramSection(const std::string& section, std::string& key, std::string& value) const {
				// Required field
				// Verified command exists
				// if (config.find("command") == config.end() || config["command"].empty())
				// 	throw std::runtime_error("[" + section + "] command is required");

				// Command
				if (key == "command" && value.empty())
					throw std::runtime_error("[" + section + "] command must exists");

				// Boolean values
				if (key == "autostart" && !isValidBool(value))
					throw std::runtime_error("[" + section + "] autostart must be true or false");

				if (key == "stopasgroup" && !isValidBool(value))
					throw std::runtime_error("[" + section + "] stopasgroup must be true or false");

				if (key == "killasgroup" && !isValidBool(value))
					throw std::runtime_error("[" + section + "] killasgroup must be true or false");

				if (key == "redirect_stderr" && !isValidBool(value))
					throw std::runtime_error("[" + section + "] redirect_stderr must be true or false");

				// Numeric values
				if (key == "numprocs" && !isValidNumber(value, 1, 1000))
					throw std::runtime_error("[" + section + "] numprocs must be between 1 and 1000");

				if (key == "priority" && !isValidNumber(value, 0, 999))
					throw std::runtime_error("[" + section + "] priority must be between 0 and 999");

				if (key == "startsecs" && !isValidNumber(value, 0, 3600))
					throw std::runtime_error("[" + section + "] startsecs must be between 0 and 3600");

				if (key == "startretries" && !isValidNumber(value, 0, 100))
					throw std::runtime_error("[" + section + "] startretries must be between 0 and 100");

				if (key == "stopwaitsecs" && !isValidNumber(value, 1, 3600))
					throw std::runtime_error("[" + section + "] stopwaitsecs must be between 1 and 3600");

				// Special validations
				if (key == "autorestart" && !isValidAutorestart(value))
					throw std::runtime_error("[" + section + "] autorestart must be false, unexpected or true");

				if (key == "exitcodes" && !isValidExitCodes(value))
					throw std::runtime_error("[" + section + "] exitcodes must be comma-separated numbers between 0 and 255");

				if (key == "stopsignal" && !isValidSignal(value))
					throw std::runtime_error("[" + section + "] stopsignal must be a valid signal (TERM, HUP, INT, QUIT, KILL, USR1, USR2)");

				// Path validation
				if (key == "directory" && !isValidPath(value, true))
					throw std::runtime_error("[" + section + "] directory path is invalid");

				// std_out, std_err: NONE, AUTO
				// AUTO = childlogdir

				// Umask validation
				if (key == "umask" && !value.empty() && !isValidUmask(value))
					throw std::runtime_error("[" + section + "] umask must be in octal format (e.g., 022)");
			}

		#pragma endregion

		#pragma region "Group"

			// void ConfigParser::validateGroupSection(const std::string& section, std::string& key, std::string& value) const {
			// 	// Required field
			// 	if (config.find("programs") == config.end() || config["programs"].empty())
			// 		errors += "[" + section + "] programs is required\n";

			// 	// Priority validation
			// 	if (!isValidNumber(config["priority"], 0, 999))
			// 		errors += "[" + section + "] priority must be between 0 and 999\n";

			// 	// Validate that referenced programs exist
			// 	std::string programs = config["programs"];
			// 	std::istringstream ss(programs);
			// 	std::string program;

			// 	while (std::getline(ss, program, ',')) {
			// 		program = trim(program);
			// 		std::string programSection = "program:" + program;
			// 		if (!hasSection(programSection))
			// 			errors += "[" + section + "] references non-existent program: " + program + "\n";
			// 	}
			// }

		#pragma endregion

		#pragma region "Unix Server"

			// void ConfigParser::validateUnixHttpServerSection(const std::string& section, std::string& key, std::string& value) const {
			// 	// Required field
			// 	// Can be created
			// 	if (config.find("file") == config.end() || config["file"].empty())
			// 		errors += "[" + section + "] file is required\n";

			// 	// Permission validation
			// 	if (!config["chmod"].empty() && !isValidUmask(config["chmod"]))
			// 		errors += "[" + section + "] chmod must be in octal format (e.g., 700)\n";
			// }

		#pragma endregion

		#pragma region "Inet Server"

			// void ConfigParser::validateInetHttpServerSection(const std::string& section, std::string& key, std::string& value) const {
			// 	// Required field
			// 	if (config.find("port") == config.end() || config["port"].empty())
			// 		errors += "[" + section + "] port is required\n";

			// 	// Port validation
			// 	if (!isValidNumber(config["port"], 1, 65535))
			// 		errors += "[" + section + "] port must be between 1 and 65535\n";
			// }

		#pragma endregion

	#pragma endregion

	#pragma region "Validate"

		void ConfigParser::validate(const std::string& section, std::string& key, std::string& value) const {
			std::string sectionType = SectionType(section);

			if		(sectionType == "taskmasterd")			validateTaskmasterdSection(section, key, value);
			else if	(sectionType == "program:")				validateProgramSection(section, key, value);
			// else if	(sectionType == "unix_http_server")		validateUnixHttpServerSection(section);
			// else if	(sectionType == "inet_http_server")		validateInetHttpServerSection(section);
			// else if	(sectionType == "group:")				validateGroupSection(section);
		}

	#pragma endregion

#pragma endregion
