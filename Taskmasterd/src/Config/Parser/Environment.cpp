/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Environment.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/28 12:25:58 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/03 19:05:34 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Config.hpp"

	#include <algorithm>														// std::all_of()
	#include <regex>															// regex_match()
	#include <iostream>															// std::cerr
	#include <iomanip>															// std::setw(), std::setfill()

#pragma endregion

#pragma region "Expanse"

	#pragma region "Format"

		std::string ConfigParser::environment_apply_format(const std::string& value, const std::string& modifier) const {
			if (modifier.empty() || modifier[0] != '*') return (value);
			std::string fmt = modifier.substr(1);

			// String
			if (fmt == "upper") return (toUpper(value));
			if (fmt == "lower") return (toLower(value));

			// Numeric
			try {
				int num = std::stoi(value);

				if (fmt == "d")																						return (std::to_string(num));
				if (fmt == "x")		{ std::stringstream ss; 	ss << std::hex << num;								return (ss.str()); }
				if (fmt == "#x")	{ std::stringstream ss; 	ss << "0x" << std::hex << num;						return (ss.str()); }
				if (fmt == "X")		{ std::stringstream ss;		ss << std::hex << std::uppercase << num;			return (ss.str()); }
				if (fmt == "#X")	{ std::stringstream ss; 	ss << "0X" << std::hex << std::uppercase << num;	return (ss.str()); }
				if (fmt == "o")		{ std::stringstream ss;		ss << std::oct << num;								return (ss.str()); }

				if (fmt.length() >= 2 && fmt.back() == 'd') {
					std::string padding = fmt.substr(0, fmt.length() - 1);
					if (std::all_of(padding.begin(), padding.end(), ::isdigit)) {
						int width = std::stoi(padding);
						std::stringstream ss;
						if (padding[0] == '0')	ss << std::setw(width) << std::setfill('0') << num;
						else					ss << std::setw(width) << std::setfill(' ') << num;
						return (ss.str());
					}
				}
			} catch (...) { return (value); }

			return (value);
		}

	#pragma endregion

	#pragma region "Substring"

		std::string ConfigParser::environment_apply_substring(const std::string& value, const std::string& modifier) const {
			size_t colon = modifier.find(':');

			if (modifier[0] == ' ' && modifier[1] == '-') {							// ${VAR: -n}
				try {
					int count = std::stoi(modifier.substr(2));
					if (count >= (int)value.length()) return (value);
					return (value.substr(value.length() - count));
				} catch (...) { return (value); }
			}

			try {
				if (colon == std::string::npos) {									// ${VAR:n}
					int start = std::stoi(modifier);
					if (start < 0 || start >= (int)value.length()) return ("");
					return (value.substr(start));
				} else {															// ${VAR:n:n}
					int start = std::stoi(modifier.substr(0, colon));
					int len   = std::stoi(modifier.substr(colon + 1));
					if (start < 0 || start >= (int)value.length()) return ("");
					return (value.substr(start, len));
				}
			} catch (...) { return (value); }
		}

	#pragma endregion

	#pragma region "Expression"

		std::string ConfigParser::environment_expand_expr(std::map<std::string, std::string>& env, const std::string& var_expr) const {
			size_t		colon = var_expr.find(':');
			std::string	var_name = (colon == std::string::npos) ? var_expr : var_expr.substr(0, colon);
			std::string	modifier = (colon == std::string::npos) ? "" : var_expr.substr(colon + 1);

			auto it = env.find(var_name);
			std::string	value;

			// Do not expand to "" if NUMPROCS or PROCESS_NUM are not set in the environment (they will be expanded later during program instance creation)
			if (it != env.end()) value = it->second;
			else if	(section_type(currentSection) == "program:" && (var_name == "PROCESS_NUM" || var_name == "EX_PROCESS_NUM")) return "${EX_" + var_expr + "}";
			else value = "";

			if (modifier.empty()) 									return (value);												// No modifier
			if (modifier.substr(0, 1) == "-")						return (value.empty() ? modifier.substr(1) : value);		// ${VAR:-default}
			if (modifier.substr(0, 1) == "+")						return (value.empty() ? "" : modifier.substr(1));			// ${VAR:+value}
			if (std::isdigit(modifier[0]) || modifier[0] == ' ')	return (environment_apply_substring(value, modifier));		// ${VAR:substring}
			if (modifier[0] == '*')									return (environment_apply_format(value, modifier));			// ${VAR:*format}

			return (value);
		}

	#pragma endregion

	#pragma region "Expand"

		std::string ConfigParser::environment_expand(std::map<std::string, std::string>& env, const std::string& line, std::string split) const {
			std::string	result;
			char		quoteChar = 0;
			bool		escaped = false;

			for (size_t i = 0; i < line.length(); ++i) {
				char c = line[i];

				if (escaped)								{ escaped = false; result += c;		continue; }
				if (c == '\\')								{ escaped = true;					continue; }
				if (!quoteChar && (c == '"' || c == '\''))	{ quoteChar = c;					continue; }
				if (quoteChar && c == quoteChar)			{ quoteChar = 0;					continue; }

				if (quoteChar != '\'' && c == '$') {
					if (i + 1 < line.length() && line[i + 1] == '{') {							// ${VAR}
						size_t	start = i + 2;
						size_t	end = start;

						while (end < line.length() && line[end] != '}') ++end;

						if (end < line.length()) {
							std::string var_expr = line.substr(start, end - start);
							result += environment_expand_expr(env, var_expr); 
							i = end;															continue;
						}
					} else {																	// $VAR
						size_t	start = i + 1;
						size_t	end = start;

						while (end < line.length() && (isalnum(static_cast<unsigned char>(line[end])) || line[end] == '_')) ++end;

						if (end > start) {
							std::string var_name = line.substr(start, end - start);
							auto it = env.find(var_name);
							if (it != env.end()) result += it->second;
							else if (section_type(currentSection) == "program:" && (var_name == "PROCESS_NUM" || var_name == "EX_PROCESS_NUM")) result += "$EX_PROCESS_NUM";
							i = end - 1; continue;
						}
					}
				}

				if (!quoteChar && !split.empty() && split.find(c) != std::string::npos)	result +='\n';
				else																	result += c;
			}

			if (quoteChar || escaped) throw std::runtime_error("[" + currentSection + "] unclosed quote or unfinished escape sequence");

			return (result);
		}

	#pragma endregion

#pragma endregion

#pragma region "Validate"

	bool ConfigParser::environment_validate(const std::string& env_string) const {
		static const std::regex env_regex(R"(^([a-zA-Z_][a-zA-Z0-9_]*)=(.+)$)");

		std::string current;
		for (char c : env_string) {
			if (c == '\n') {
				std::string trimmed = trim(current);
				if (!trimmed.empty() && !std::regex_match(trimmed, env_regex)) {
					return false;
				}
				current.clear();
			} else {
				current += c;
			}
		}

		std::string trimmed = trim(current);
		return (trimmed.empty() || std::regex_match(trimmed, env_regex));
	}

#pragma endregion

#pragma region "Manage"

	#pragma region "Print"

		void ConfigParser::environment_print(const std::map<std::string, std::string>& env) const {
			for (const auto& pair : env) {
				std::cerr << pair.first << "=" << pair.second << "\n";
			}
		}

	#pragma endregion

	#pragma region "Del"

		void ConfigParser::environment_del(std::map<std::string, std::string>& env, const std::string& key) {
			if (!key.empty()) env.erase(key);
		}

	#pragma endregion

	#pragma region "Add"

		void ConfigParser::environment_add(std::map<std::string, std::string>& env, const std::string& key, const std::string& value) {
			if (!key.empty() && !value.empty()) env[key] = value;
		}

		void ConfigParser::environment_add(std::map<std::string, std::string>& env, const std::map<std::string, std::string>& src, bool overwrite) {
			for (const auto& [key, value] : src) {
				if (overwrite || env.find(key) == env.end()) env[key] = value;
			}
		}

	#pragma endregion

	#pragma region "Add Batch"

		void ConfigParser::environment_add_batch(std::map<std::string, std::string>& env, const std::string& batch) {
			if (batch.empty()) return ;

			std::vector<std::string>	pairs;
			std::string					current;
			char						quoteChar = 0;
			bool						escaped = false;

			for (char c : batch) {
				if (escaped)								{ current += c; escaped = false; 				continue; }
				if (c == '\\')								{ escaped = true;								continue; }
				if (!quoteChar && (c == '"' || c == '\''))	{ quoteChar = c;								continue; }
				if (quoteChar && c == quoteChar)			{ quoteChar = 0;								continue; }
				if (!quoteChar && c == '\n')				{ pairs.push_back(current); current.clear(); 	continue; }

				current += c;
			}
			if (!current.empty()) pairs.push_back(current);

			for (const std::string& pair : pairs) {
				size_t pos = pair.find('=');
				if (pos != std::string::npos) {
					std::string	key   = trim(pair.substr(0, pos));
					std::string	value = trim(pair.substr(pos + 1));
					env[key] = value;
				}
			}
		}

	#pragma endregion

	#pragma region "Get"

		std::string ConfigParser::environment_get(const std::map<std::string, std::string>& env, const std::string& key) const {
			if (key.empty()) return ("");

			auto it = env.find(key);
   			return ((it != env.end()) ? it->second : "");
		}

	#pragma endregion

	#pragma region "Clone"
		
		void ConfigParser::environment_clone(std::map<std::string, std::string>& env, const std::map<std::string, std::string>& src) {
			if (env != src) env = src;
		}

	#pragma endregion

	#pragma region "Initialize"

		extern char **environ;

		void ConfigParser::environment_initialize(std::map<std::string, std::string>& env) {
			for (char **sys_env = environ; *sys_env != nullptr; ++sys_env) {
				std::string entry(*sys_env);
				auto pos = entry.find('=');
				if (pos != std::string::npos) {
					std::string key   = entry.substr(0, pos);
					std::string value = entry.substr(pos + 1);
					env[key] = value;
				}
			}
		}

	#pragma endregion

#pragma endregion
