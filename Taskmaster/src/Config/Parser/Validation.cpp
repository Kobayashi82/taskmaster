/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Validation.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 11:32:25 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/31 00:13:52 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Parser.hpp"

	#include <unistd.h>															// access()
	#include <iostream>															// std::cerr
	#include <filesystem>														// std::filesystem::path(), std::filesystem::is_directory(), std::filesystem::exists()
	#include <pwd.h>															// struct passwd, getpwnam()
	#include <grp.h>															// struct group, getgrnam()
	#include <regex>															// std::regex_match()
	#include <climits>															// INT_MAX

#pragma endregion

#pragma region "Helpers"

	#pragma region "Bool"

		bool ConfigParser::valid_bool(const std::string& value) const {
			std::string lower = toLower(value);
			return (lower == "true" || lower == "false" || lower == "1" || lower == "0" || lower == "yes" || lower == "no");
		}

	#pragma endregion

	#pragma region "Number"

		bool ConfigParser::valid_number(const std::string& value, long min, long max) const {
			if (value.empty()) return (false);

			char	*end;
			long	num = std::strtol(value.c_str(), &end, 10);

			return (*end == '\0' && num >= min && num <= max);
		}

	#pragma endregion

	#pragma region "Path"

		bool ConfigParser::valid_path(const std::string& value, bool is_directory, bool allow_auto, bool allow_none, bool allow_syslog) const {
			std::string fullPath;

			if (value.empty())											return (false);
			if (allow_none && toLower(value) == "none")					return (true);
			if (allow_auto && toLower(value) == "auto")					return (true);
			if (allow_syslog && toLower(value) == "syslog")				return (true);
			else if (is_directory && toLower(value) == "do not change")	fullPath = expand_path(".");
			else														fullPath = expand_path(value);
			if (fullPath.empty())										return (false);

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

		bool ConfigParser::valid_signal(const std::string& value) const {
			std::set<std::string> validSignals = { "1", "HUP", "SIGHUP", "2", "INT", "SIGINT", "3", "QUIT", "SIGQUIT", "9", "KILL", "SIGKILL", "15", "TERM", "SIGTERM", "10", "USR1", "SIGUSR1", "12", "USR2", "SIGUSR2" };

			return (validSignals.count(toUpper(value)) > 0);
		}

	#pragma endregion

	#pragma region "Exit Code"

		bool ConfigParser::valid_code(const std::string& value) const {
			if (value.empty()) return (true);

			std::istringstream ss(value);
			std::string code;

			while (std::getline(ss, code, ',')) {
				code = trim(code);
				if (!valid_number(code, 0, 255)) return (false);
			}

			return (true);
		}

	#pragma endregion

	#pragma region "Log Level"

		bool ConfigParser::valid_loglevel(const std::string& value) const {
			std::set<std::string> validSignals = { "0", "DEBUG", "1", "INFO", "2", "WARN", "WARNING", "3", "ERROR", "4", "CRITICAL" };

			return (validSignals.count(toUpper(value)) > 0);
		}

	#pragma endregion

	#pragma region "Auto Restart"

		bool ConfigParser::valid_autorestart(const std::string& value) const {
			std::string lower = toLower(value);

			return (lower == "false" || lower == "unexpected" || lower == "true" || lower == "yes" || lower == "no" || lower == "0" || lower == "1");
		}

	#pragma endregion

	#pragma region "Umask"

		bool ConfigParser::valid_umask(const std::string& value) const {
			if (value.length() < 1 || value.length() > 4) return (false);

			for (size_t i = 0; i < value.length(); ++i) {
				if (value[i] < '0' || value[i] > '7') return (false);
			}

			return (true);
		}

	#pragma endregion

	#pragma region "User"

		bool ConfigParser::valid_user(const std::string& value) const {
			if (value.empty())						return (false);
			if (toLower(value) == "do not switch")	return (true);

			struct passwd *pw = nullptr;
			char *endptr = nullptr; errno = 0;
			long uid = std::strtol(value.c_str(), &endptr, 10);
			if (!*endptr && errno == 0 && uid >= 0 && uid <= INT_MAX)		pw = getpwuid(static_cast<uid_t>(uid));
			else															pw = getpwnam(value.c_str());

			if (!pw || (pw->pw_shell && std::string(pw->pw_shell).find("nologin") != std::string::npos)) return (false);

			return (true);
		}

	#pragma endregion

	#pragma region "Chown"

		bool ConfigParser::valid_chown(const std::string& value) const {
			if (value.empty()) return (false);

			size_t colon_pos = value.find(':');

			if (colon_pos == std::string::npos) {
				if (toLower(value) == "do not switch")					return (false);
				if (!valid_user(value))									return (false);

				return (true);
			}

			std::string username = value.substr(0, colon_pos);
			std::string group    = value.substr(colon_pos + 1);

			if (toLower(value) == "do not switch")						return (false);
			if (username.empty() || !valid_user(username))				return (false);
			if (group.empty())											return (false);

			struct passwd *pw = nullptr;
			char *endptr = nullptr; errno = 0;
			long uid = std::strtol(username.c_str(), &endptr, 10);
			if (!*endptr && errno == 0 && uid >= 0 && uid <= INT_MAX)	pw = getpwuid(static_cast<uid_t>(uid));
			else														pw = getpwnam(username.c_str());
			if (!pw)													return false;

			struct group *gr = nullptr;
			endptr = nullptr; errno = 0;
			long gid = std::strtol(group.c_str(), &endptr, 10);
			if (!*endptr && errno == 0 && gid >= 0 && gid <= INT_MAX)	gr = getgrgid(static_cast<gid_t>(gid));
			else														gr = getgrnam(group.c_str());
			if (!gr)													return (false);

			if (pw->pw_gid == gr->gr_gid)								return (true);
			for (char **member = gr->gr_mem; *member != nullptr; ++member) {
				if (username == *member) return (true);
			}

			return (false);
		}

	#pragma endregion

	#pragma region "Password"

		bool ConfigParser::valid_password(const std::string& value) const {
			if (value.empty()) return (false);

			if (toLower(value.substr(0, 5)) == "{sha}") {
				std::string hash = value.substr(5);
				if (hash.length() != 40) return (false);

				for (char c : hash) {
					if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) return (false);
				}
			}

			return (true);
		}

	#pragma endregion

	#pragma region "Port"

		bool ConfigParser::valid_port(const std::string& value) const {
			size_t colon_pos = value.find(':');
			if (colon_pos == std::string::npos)						return (false);

			std::string host = value.substr(0, colon_pos);
			std::string port = value.substr(colon_pos + 1);

			if (port.empty())										return (false);

			char *endptr = nullptr; errno = 0;
			long nport = std::strtol(port.c_str(), &endptr, 10);
			if (*endptr || errno || nport < 1 || nport > 65535)		return (false);

			if (host.empty() || host == "*")						return (true);
			return (std::regex_match(host, std::regex(R"(^[a-zA-Z0-9][a-zA-Z0-9\.-]*$)")));
		}

	#pragma endregion

	#pragma region "Server URL"

		bool ConfigParser::valid_serverurl(const std::string &value) const {
			if (value.empty())				return (false);
			if (toLower(value) == "auto")	return (true);
		
			static const std::regex pattern(R"(^(https?://[^\s/:]+(:\d+)?(/[^\s]*)?|unix:.+)$)", std::regex::icase);

			std::smatch match;
			if (!std::regex_match(value, match, pattern)) return (false);
			
			if (match[2].matched) {
				const std::string	port_str = match[2].str().substr(1);
				char				*endptr = nullptr;

				long port = std::strtol(port_str.c_str(), &endptr, 10);

				if (*endptr || port < 0 || port > 65535) return (false);
			}

			return (true);
		}

	#pragma endregion

#pragma endregion

#pragma region "Validators"

	#pragma region "Taskmasterd"

		void ConfigParser::validate_taskmasterd(const std::string& section, const std::string& key, std::string& value) const {
			// Boolean values
			if (key == "nodaemon" && !valid_bool(value))
				throw std::runtime_error("[" + section + "] nodaemon: must be true or false");

			if (key == "silent" && !valid_bool(value))
				throw std::runtime_error("[" + section + "] silent: must be true or false");

			if (key == "strip_ansi" && !valid_bool(value))
				throw std::runtime_error("[" + section + "] strip_ansi: must be true or false");

			if (key == "nocleanup" && !valid_bool(value))
				throw std::runtime_error("[" + section + "] nocleanup: must be true or false");

			if (key == "logfile_syslog" && !valid_bool(value))
				throw std::runtime_error("[" + section + "] logfile_syslog: must be true or false");

			// Numeric values
			if (key == "logfile_maxbytes") {
				long bytes = parse_size(value);
				if (bytes == -1 || !valid_number(std::to_string(bytes), 0, 1024 * 1024 * 1024))
					throw std::runtime_error("[" + section + "] logfile_maxbytes: must be a value between 0 bytes and 1024 MB");
			}

			if (key == "logfile_backups" && !valid_number(value, 0, 1000))
				throw std::runtime_error("[" + section + "] logfile_backups: must be a value between 0 and 1000");

			if (key == "loglevel" && !valid_loglevel(value))
				throw std::runtime_error("[" + section + "] loglevel: must be one of: DEBUG, INFO, WARNING, ERROR, CRITICAL");

			if (key == "minfds") {
				if (!valid_number(value, 1, 65535))
					throw std::runtime_error("[" + section + "] minfds: must be a value between 1 and 65535");
				if (check_fd_limit(static_cast<uint16_t>(std::stoul(value))))
					throw std::runtime_error("[" + section + "] minfds: limit could not be applied (system limit too low or insufficient permissions)");
			}

			if (key == "minprocs") {
				if (!valid_number(value, 1, 10000))
					throw std::runtime_error("[" + section + "] minprocs: must be a value between 1 and 10000");
				if (check_process_limit(static_cast<uint16_t>(std::stoul(value))))
					throw std::runtime_error("[" + section + "] minprocs: limit could not be applied (system limit too low or insufficient permissions)");
			}

			// Path validation
			if (key == "directory" && !valid_path(value, true))
				throw std::runtime_error("[" + section + "] directory: path is invalid");

			if (key == "logfile" && !valid_path(value, false, false, true, true))
				throw std::runtime_error("[" + section + "] logfile: path is invalid");

			if (key == "pidfile" && !valid_path(value))
				throw std::runtime_error("[" + section + "] pidfile: path is invalid");

			if (key == "childlogdir" && !valid_path(value, true))
				throw std::runtime_error("[" + section + "] childlogdir: path is invalid");

			// User validation
			if (key == "user" && !valid_user(value))
				throw std::runtime_error("[" + section + "] user: invalid user");

			// Umask validation
			if (key == "umask" && !valid_umask(value))
				throw std::runtime_error("[" + section + "] umask: must be in octal format");

			// Environment validation
			if (key == "environment" && !environment_validate(value))
				throw std::runtime_error("[" + section + "] environment: invalid variable format");
		}

	#pragma endregion

	#pragma region "Program"

		void ConfigParser::validate_program(const std::string& section, std::string& key, std::string& value) const {
			// Command
			if (key == "command") {
				std::string command;
				if (!command_executable(value, command))	throw std::runtime_error("[" + section + "] command: must be a valid executable");
			}

			if (key == "process_name") {
				std::string numprocs = get_value(currentSection, "numprocs");
				if (numprocs != "1" && value.find("${PROCESS_NUM") == std::string::npos && value.find("$PROCESS_NUM") == std::string::npos)
					throw std::runtime_error("[" + section + "] process_name: must include $PROCESS_NUM when 'numprocs' is greater than 1");
			}

			// Boolean values
			if (key == "autostart" && !valid_bool(value))
				throw std::runtime_error("[" + section + "] autostart: must be true or false");

			if (key == "stopasgroup" && !valid_bool(value))
				throw std::runtime_error("[" + section + "] stopasgroup: must be true or false");

			if (key == "killasgroup" && !valid_bool(value))
				throw std::runtime_error("[" + section + "] killasgroup: must be true or false");

			if (key == "redirect_stderr" && !valid_bool(value))
				throw std::runtime_error("[" + section + "] redirect_stderr: must be true or false");

			if (key == "stdout_syslog" && !valid_bool(value))
				throw std::runtime_error("[" + section + "] stdout_syslog: must be true or false");

			if (key == "stderr_syslog" && !valid_bool(value))
				throw std::runtime_error("[" + section + "] stderr_syslog: must be true or false");

			// Numeric values
			if (key == "stdout_logfile_maxbytes") {
				long bytes = parse_size(value);
				if (bytes == -1 || !valid_number(std::to_string(bytes), 0, 1024 * 1024 * 1024))
					throw std::runtime_error("[" + section + "] stdout_logfile_maxbytes: must be a value between 0 bytes and 1024 MB");
			}
			if (key == "stdout_logfile_backups" && !valid_number(value, 0, 1000))
				throw std::runtime_error("[" + section + "] stdout_logfile_backups: must be a value between 0 and 1000");

			if (key == "stderr_logfile_maxbytes") {
				long bytes = parse_size(value);
				if (bytes == -1 || !valid_number(std::to_string(bytes), 0, 1024 * 1024 * 1024))
					throw std::runtime_error("[" + section + "] stderr_logfile_maxbytes: must be a value between 0 bytes and 1024 MB");
			}
			if (key == "stderr_logfile_backups" && !valid_number(value, 0, 1000))
				throw std::runtime_error("[" + section + "] stderr_logfile_backups: must be a value between 0 and 1000");

			if (key == "numprocs" && !valid_number(value, 1, 1000))
				throw std::runtime_error("[" + section + "] numprocs: must be a value between 1 and 1000");
			if (key == "numprocs" && value != "1") {
				std::string process_name = get_value(currentSection, "process_name");
				if (process_name.empty() || (process_name.find("${PROCESS_NUM") == std::string::npos && process_name.find("$PROCESS_NUM") == std::string::npos))
					throw std::runtime_error("[" + section + "] numprocs: 'process_name' must include $PROCESS_NUM when 'numprocs' is greater than 1");
			}

			if (key == "priority" && !valid_number(value, 0, 999))
				throw std::runtime_error("[" + section + "] priority: must be a value between 0 and 999");

			if (key == "startsecs" && !valid_number(value, 0, 3600))
				throw std::runtime_error("[" + section + "] startsecs: must be a value between 0 and 3600");

			if (key == "startretries" && !valid_number(value, 0, 100))
				throw std::runtime_error("[" + section + "] startretries: must be a value between 0 and 100");

			if (key == "stopwaitsecs" && !valid_number(value, 1, 3600))
				throw std::runtime_error("[" + section + "] stopwaitsecs: must be a value between 1 and 3600");

			// Special validations
			if (key == "autorestart" && !valid_autorestart(value))
				throw std::runtime_error("[" + section + "] autorestart: must be false, unexpected or true");

			if (key == "exitcodes" && !valid_code(value))
				throw std::runtime_error("[" + section + "] exitcodes: must be comma-separated numbers between 0 and 255");

			if (key == "stopsignal" && !valid_signal(value))
				throw std::runtime_error("[" + section + "] stopsignal: must be a valid signal (HUP, INT, QUIT, KILL, TERM, USR1, USR2)");

			// User validation
			if (key == "user" && !valid_user(value))
				throw std::runtime_error("[" + section + "] user: invalid user");

			// Path validation
			if (key == "directory" && !valid_path(value, true))
				throw std::runtime_error("[" + section + "] directory: path is invalid");

			if (key == "stdout_logfile" && !valid_path(value, false, true, true, true))
				throw std::runtime_error("[" + section + "] stdout_logfile: path is invalid");

			if (key == "stderr_logfile" && !valid_path(value, false, true, true, true))
				throw std::runtime_error("[" + section + "] stderr_logfile: path is invalid");

			// Umask validation
			if (key == "umask" && !valid_umask(value))
				throw std::runtime_error("[" + section + "] umask: must be in octal format");

			// Serverurl validation
			if (key == "serverurl" && !valid_serverurl(value))
				throw std::runtime_error("[" + section + "] serverurl: invalid format");

			// Environment validation
			if (key == "environment" && !environment_validate(value))
				throw std::runtime_error("[" + section + "] environment: invalid variable format");
		}

	#pragma endregion

	#pragma region "Group"

		void ConfigParser::validate_group(const std::string& section, std::string& key, std::string& value) const {
			if (key == "priority" && !valid_number(value, 0, 999))
				throw std::runtime_error("[" + section + "] priority: must be a value between 0 and 999");
		}

	#pragma endregion

	#pragma region "Unix Server"

		void ConfigParser::validate_unix_server(const std::string& section, std::string& key, std::string& value) const {
			// Path validation
			if (key == "file" && !valid_path(value, false))
				throw std::runtime_error("[" + section + "] file: path is invalid");

			// Chmod validation
			if (key == "chmod" && !valid_umask(value))
				throw std::runtime_error("[" + section + "] chmod: must be in octal format");

			// Chown validation
			if (key == "chown" && !valid_chown(value))
				throw std::runtime_error("[" + section + "] chown: invalid user or group");

			// Password validation
			if (key == "password" && !valid_password(value))
				throw std::runtime_error("[" + section + "] password: invalid SHA format");
		}

	#pragma endregion

	#pragma region "Inet Server"

		void ConfigParser::validate_inet_server(const std::string& section, std::string& key, std::string& value) const {
			// Port validation
			if (key == "port" && !valid_port(value))
				throw std::runtime_error("[" + section + "] port: must be a valid TCP host:port");

			// Password validation
			if (key == "password" && !valid_password(value))
				throw std::runtime_error("[" + section + "] password: invalid SHA format");
		}

	#pragma endregion

	#pragma region "Options"

		int ConfigParser::validate_options(ConfigOptions& Options) const {
			std::string errors;

			if (Options.options.find_first_of('c') != std::string::npos) {
				if (!Options.configuration.empty() && expand_path(Options.configuration, "", true, false).empty())
					errors += "configuration: cannot open config file\n";
			}

			if (Options.options.find_first_of('u') != std::string::npos) {
				try { validate_taskmasterd("", "user", Options.user); }
				catch (const std::exception& e) { errors += std::string(e.what()).substr(3) + "\n"; }
			}

			if (Options.options.find_first_of('m') != std::string::npos) {
				try { validate_taskmasterd("", "umask", Options.umask); }
				catch (const std::exception& e) { errors += std::string(e.what()).substr(3) + "\n"; }
			}

			if (Options.options.find_first_of('d') != std::string::npos) {
				try { validate_taskmasterd("", "directory", Options.directory); }
				catch (const std::exception& e) { errors += std::string(e.what()).substr(3) + "\n"; }
			}

			if (Options.options.find_first_of('l') != std::string::npos) {
				try { validate_taskmasterd("", "logfile", Options.logfile); }
				catch (const std::exception& e) { errors += std::string(e.what()).substr(3) + "\n"; }
			}

			if (Options.options.find_first_of('y') != std::string::npos) {
				try { validate_taskmasterd("", "logfile_maxbytes", Options.logfile_maxbytes); }
				catch (const std::exception& e) { errors += std::string(e.what()).substr(3) + "\n"; }
			}

			if (Options.options.find_first_of('z') != std::string::npos) {
				try { validate_taskmasterd("", "logfile_backups", Options.logfile_backups); }
				catch (const std::exception& e) { errors += std::string(e.what()).substr(3) + "\n"; }
			}

			if (Options.options.find_first_of('e') != std::string::npos) {
				try { validate_taskmasterd("", "loglevel", Options.loglevel); }
				catch (const std::exception& e) { errors += std::string(e.what()).substr(3) + "\n"; }
			}

			if (Options.options.find_first_of('j') != std::string::npos) {
				try { validate_taskmasterd("", "pidfile", Options.pidfile); }
				catch (const std::exception& e) { errors += std::string(e.what()).substr(3) + "\n"; }
			}

			if (Options.options.find_first_of('q') != std::string::npos) {
				try { validate_taskmasterd("", "childlogdir", Options.childlogdir); }
				catch (const std::exception& e) { errors += std::string(e.what()).substr(3) + "\n"; }
			}

			if (Options.options.find_first_of('a') != std::string::npos) {
				try { validate_taskmasterd("", "minfds", Options.minfds); }
				catch (const std::exception& e) { errors += std::string(e.what()).substr(3) + "\n"; }
			}

			if (Options.options.find_first_of('p') != std::string::npos) {
				try { validate_taskmasterd("", "minprocs", Options.minprocs); }
				catch (const std::exception& e) { errors += std::string(e.what()).substr(3) + "\n"; }
			}

			if (!errors.empty()) { std::cerr << Options.fullName << ": invalid options: \n\n" <<  errors; return (2); }

			return (0);
		}

	#pragma endregion

#pragma endregion

#pragma region "Validate"

	void ConfigParser::validate(const std::string& section, std::string& key, std::string& value) const {
		std::string sectionType = section_type(section);

		if		(sectionType == "taskmasterd")			validate_taskmasterd(section, key, value);
		else if	(sectionType == "program:")				validate_program(section, key, value);
		else if	(sectionType == "group:")				validate_group(section, key, value);
		else if	(sectionType == "unix_http_server")		validate_unix_server(section, key, value);
		else if	(sectionType == "inet_http_server")		validate_inet_server(section, key, value);
	}

#pragma endregion
