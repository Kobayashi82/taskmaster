/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Environment.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/28 12:25:58 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/08 17:41:02 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Utils/Utils.hpp"

	#include <algorithm>														// std::all_of()
	#include <regex>															// regex_match()
	#include <iostream>															// std::cerr
	#include <iomanip>															// std::setw(), std::setfill()

#pragma endregion

#pragma region "Expanse"

	#pragma region "Format"

		std::string Utils::environment_apply_format(const std::string& value, const std::string& modifier) {
			if (modifier.empty()) return (value);
			std::string fmt = modifier;

			// String
			if (fmt == "^^") return (toUpper(value));
			if (fmt == ",,") return (toLower(value));
			if (fmt == "^") {
				if (value.empty()) return (value);
				std::string result = value;
				result[0] = std::toupper(result[0]);
				return (result);
			}
			if (fmt == ",") {
				if (value.empty()) return (value);
				std::string result = value;
				result[0] = std::tolower(result[0]);
				return (result);
			}
			
			// Path
			if (fmt == "~") {
				size_t slash = value.rfind('/');
				return ((slash != std::string::npos) ? value.substr(slash + 1) : value);
			}

			// Numeric
			try {
				int num = std::stoi(value);

				if (fmt == "d")																						return (((num > 0) ? "+" : "-") + std::to_string(num));
				if (fmt == "x")		{ std::stringstream ss; 	ss << std::hex << num;								return (ss.str()); }
				if (fmt == "X")		{ std::stringstream ss;		ss << std::hex << std::uppercase << num;			return (ss.str()); }
				if (fmt == "#x")	{ std::stringstream ss; 	ss << "0x" << std::hex << num;						return (ss.str()); }
				if (fmt == "#X")	{ std::stringstream ss; 	ss << "0X" << std::hex << std::uppercase << num;	return (ss.str()); }
				if (fmt == "o")		{ std::stringstream ss;		ss << std::oct << num;								return (ss.str()); }

				if (fmt.length() >= 2 && fmt.back() == 'd') {
					std::string padding = fmt.substr(0, fmt.length() - 1);
					if (isDigit(padding)) {
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

		std::string Utils::environment_apply_substring(const std::string& value, const std::string& modifier) {
			size_t colon = modifier.find(':');

			if (modifier[0] == ' ' && modifier[1] == '-') {
				try {
					if (colon == std::string::npos) {								// ${VAR: -n}
						int count = std::stoi(modifier.substr(2));
						if (count >= (int)value.length()) return (value);
						return (value.substr(value.length() - count));
					} else {														// ${VAR: -n:n}
						int start = std::stoi(modifier.substr(2, colon));
						int len   = std::stoi(modifier.substr(colon + 1));
						int start_pos = value.length() - start;
						if (start_pos < 0) start_pos = 0;
						return (value.substr(start_pos, len));
					}
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

		std::string Utils::environment_expand_expr(std::map<std::string, std::string>& env, const std::string& var_expr) {
			if (!var_expr.empty() && var_expr[0] == '#') {																		// ${#VAR}
				auto it = env.find(var_expr.substr(1));
				return ((it != env.end()) ? std::to_string(it->second.length()) : "0");
			}

			size_t		colon = var_expr.find(':');
			std::string	var_name = (colon == std::string::npos) ? var_expr : var_expr.substr(0, colon);
			std::string	modifier = (colon == std::string::npos) ? "" : var_expr.substr(colon + 1);

			auto it = env.find(var_name);
			std::string	value;

			if (it != env.end())	value = it->second;
			else					value = "";

			if (modifier.empty()) 									return (value);												// No modifier
			if (modifier[0] == '-')									return (value.empty() ? modifier.substr(1) : value);		// ${VAR:-default}  
			if (modifier[0] == '+')									return (value.empty() ? "" : modifier.substr(1));			// ${VAR:+value}
			if (modifier.find_last_of("dxXo^,~") != std::string::npos)	return (environment_apply_format(value, modifier));			// {$VAR:02d}
			if (std::isdigit(modifier[0]) || modifier[0] == ' ')	return (environment_apply_substring(value, modifier));		// ${VAR:substring}

			return (value);
		}

	#pragma endregion

	#pragma region "Expand"

		std::string Utils::environment_expand(std::map<std::string, std::string>& env, const std::string& line) {
			std::string	result;
			char		quoteChar = 0;
			bool		escaped = false;

			for (size_t i = 0; i < line.length(); ++i) {
				char c = line[i];

				if (escaped)								{ escaped = false;	result += c;	continue; }
				if (quoteChar != '\'' && c == '\\')			{ escaped = true;	result += c;	continue; }
				if (!quoteChar && (c == '"' || c == '\''))	{ quoteChar = c;	result += c;	continue; }
				if (quoteChar && c == quoteChar)			{ quoteChar = 0;	result += c;	continue; }

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
							i = end - 1; continue;
						}
					}
				}

				result += c;
			}

			if (quoteChar || escaped) throw std::runtime_error("unclosed quote or unfinished escape sequence");

			return (result);
		}

	#pragma endregion

#pragma endregion

#pragma region "Validate"

	bool Utils::environment_validate(const std::string& env_string) {
		if (env_string.empty()) return (true);

		std::vector<std::string>	pairs;
		std::string					current;
		char						quoteChar = 0;
		bool						escaped = false;

		for (char c : env_string) {
			if (escaped)								{ escaped = false;			current += c;		continue; }
			if (quoteChar != '\'' && c == '\\')			{ escaped = true;								continue; }
			if (!quoteChar && (c == '"' || c == '\''))	{ quoteChar = c;								continue; }
			if (quoteChar && c == quoteChar)			{ quoteChar = 0;								continue; }
			if (!quoteChar && (c == '\n' || c == ','))	{ pairs.push_back(current);	current.clear();	continue; }

			current += c;
		}
		if (!current.empty()) pairs.push_back(current);

		if (quoteChar || escaped) return (false);

		for (const std::string& pair : pairs) {
			std::string trimmed = trim(pair);
			if (trimmed.empty()) continue;

			size_t plus_pos = trimmed.find("+=");
			size_t eq_pos   = trimmed.find('=');

			std::string key;
			if (plus_pos != std::string::npos)		key = trim(trimmed.substr(0, plus_pos));
			else if (eq_pos != std::string::npos)	key = trim(trimmed.substr(0, eq_pos));
			else											return (false);

			if (key.empty())								return (false);
			if (!std::isalpha(key[0]) && key[0] != '_')		return (false);

			for (size_t i = 1; i < key.length(); ++i) {
				if (!std::isalnum(key[i]) && key[i] != '_')	return (false);
			}
		}

		return (true);
	}

#pragma endregion

#pragma region "Manage"

	#pragma region "Print"

		void Utils::environment_print(const std::map<std::string, std::string>& env) {
			for (const auto& pair : env) {
				std::cerr << pair.first << "=" << pair.second << "\n";
			}
		}

	#pragma endregion

	#pragma region "Del"

		void Utils::environment_del(std::map<std::string, std::string>& env, const std::string& key) {
			if (!key.empty()) env.erase(key);
		}

	#pragma endregion

	#pragma region "Add"

		void Utils::environment_add(std::map<std::string, std::string>& env, const std::string& key, const std::string& value, bool append) {
			if (key.empty()) return;

			if (append && !value.empty()) {
				auto it = env.find(key);
				if (it != env.end())	env[key] = it->second + value;
				else					env[key] = value;
			} else if (!value.empty())	env[key] = value;
		}

		void Utils::environment_add(std::map<std::string, std::string>& env, const std::map<std::string, std::string>& src, bool overwrite) {
			for (const auto& [key, value] : src) {
				if (overwrite || env.find(key) == env.end()) env[key] = value;
			}
		}

	#pragma endregion

	#pragma region "Add Batch"

		void Utils::environment_add_batch(std::map<std::string, std::string>& env, const std::string& batch) {
			if (batch.empty()) return;

			std::vector<std::string>	pairs;
			std::string					current;
			char						quoteChar = 0;
			bool						escaped = false;

			for (char c : batch) {
				if (escaped)								{ escaped = false;			current += c;		continue; }
				if (quoteChar != '\'' && c == '\\')			{ escaped = true;								continue; }
				if (!quoteChar && (c == '"' || c == '\''))	{ quoteChar = c;								continue; }
				if (quoteChar && c == quoteChar)			{ quoteChar = 0;								continue; }
				if (!quoteChar && (c == '\n' || c == ','))	{ pairs.push_back(current);	current.clear();	continue; }

				current += c;
			}
			if (!current.empty()) pairs.push_back(current);

			for (const std::string& pair : pairs) {

				size_t plus_pos = pair.find("+=");
				if (plus_pos != std::string::npos) {
					std::string	key		= trim(pair.substr(0, plus_pos));
					std::string	value	= trim(pair.substr(plus_pos + 2));

					auto it = env.find(key);
					if (it != env.end())	env[key] = it->second + value;
					else					env[key] = value;
					continue;
				}

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

		std::string Utils::environment_get(const std::map<std::string, std::string>& env, const std::string& key) {
			if (key.empty()) return ("");

			auto it = env.find(key);
   			return ((it != env.end()) ? it->second : "");
		}

	#pragma endregion

	#pragma region "Clone"
		
		void Utils::environment_clone(std::map<std::string, std::string>& env, const std::map<std::string, std::string>& src) {
			if (env != src) env = src;
		}

	#pragma endregion

	#pragma region "Initialize"

		extern char **environ;

		void Utils::environment_initialize(std::map<std::string, std::string>& env) {
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
