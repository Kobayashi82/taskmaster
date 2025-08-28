/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Environment.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/28 12:25:58 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/28 20:43:39 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Parser.hpp"

	#include <algorithm>														// std::all_of()

#pragma endregion

#pragma region "Variables"

	extern char **environ;

#pragma endregion

#pragma region "Expanse"

	#pragma region "Format"

		std::string ConfigParser::apply_format(const std::string& value, const std::string& format) {
			if (format.empty() || format[0] != '*') return (value);

			std::string fmt = format.substr(1); // Quitar el *

			// Transformaciones de texto
			if (fmt == "upper") return (toUpper(value));
			if (fmt == "lower") return (toLower(value));

			// Formateo numérico - convertir string a número
			try {
				int num = std::stoi(value);

				if (fmt == "d") return (std::to_string(num));
				if (fmt == "x") {
					std::stringstream ss;
					ss << std::hex << num;
					return (ss.str());
				}
				if (fmt == "#x") {
					std::stringstream ss;
					ss << "0x" << std::hex << num;
					return (ss.str());
				}
				if (fmt == "X") {
					std::stringstream ss;
					ss << std::hex << std::uppercase << num;
					return (ss.str());
				}
				if (fmt == "#X") {
					std::stringstream ss;
					ss << "0X" << std::hex << std::uppercase << num;
					return (ss.str());
				}
				if (fmt == "o") {
					std::stringstream ss;
					ss << std::oct << num;
					return (ss.str());
				}
				// Formateo con padding (02d, 03d, etc.)
				if (fmt.length() >= 2 && fmt.back() == 'd') {
					std::string padding = fmt.substr(0, fmt.length() - 1);
					if (padding[0] == '0' && std::all_of(padding.begin() + 1, padding.end(), ::isdigit)) {
						int width = std::stoi(padding);
						std::stringstream ss;
						ss << std::setw(width) << std::setfill('0') << num;
						return (ss.str());
					}
				}
			} catch (...) { return (""); }

			return (value);
		}

	#pragma endregion

	#pragma region "Substring"

		std::string ConfigParser::apply_substring(const std::string& value, const std::string& substr_spec) {
			size_t colon = substr_spec.find(':');

			// ${VAR: -2} - últimos N caracteres
			if (substr_spec[0] == ' ' && substr_spec[1] == '-') {
				try {
					int count = std::stoi(substr_spec.substr(2));
					if (count >= (int)value.length()) return (value);
					return (value.substr(value.length() - count));
				} catch (...) { return (value); }
			}

			try {
				if (colon == std::string::npos) {
					// ${VAR:2} - desde posición hasta el final
					int start = std::stoi(substr_spec);
					if (start < 0 || start >= (int)value.length()) return ("");
					return (value.substr(start));
				} else {
					// ${VAR:2:3} - desde posición, N caracteres
					int start = std::stoi(substr_spec.substr(0, colon));
					int len = std::stoi(substr_spec.substr(colon + 1));
					if (start < 0 || start >= (int)value.length()) return ("");
					return (value.substr(start, len));
				}
			} catch (...) { return (value); }
		}

	#pragma endregion

	#pragma region "Expand Var"

		std::string ConfigParser::expand_variable(const std::string& var_expr, std::map<std::string, std::string>& env) {
			// ${VAR:modifier}
			size_t		colon = var_expr.find(':');
			std::string	var_name = (colon == std::string::npos) ? var_expr : var_expr.substr(0, colon);
			std::string	modifier = (colon == std::string::npos) ? "" : var_expr.substr(colon + 1);

			auto it = env.find(var_name);
			std::string value;

			// Do not expand to "" if NUMPROCS or PROCESS_NUM are not set in the environment (they will be expanded later during program instance creation)
			if (it != env.end()) value = it->second;
			else if (var_name == "NUMPROCS" || var_name == "PROCESS_NUM") return "${" + var_expr + "}";
			else value = "";

			if (modifier.empty()) return (value);

			// ${VAR:-default}
			if (modifier.substr(0, 1) == "-") return (value.empty() ? modifier.substr(1) : value);

			// ${VAR:+value}
			if (modifier.substr(0, 1) == "+") return (value.empty() ? "" : modifier.substr(1));

			// ${VAR:*format}
			if (modifier[0] == '*') return (apply_format(value, modifier));

			// ${VAR:substring}
			if (std::isdigit(modifier[0]) || modifier[0] == ' ') return (apply_substring(value, modifier));

			return (value);
		}

	#pragma endregion

	#pragma region "Expand"

		std::string ConfigParser::environment_expand(std::map<std::string, std::string>& env, std::string& str) {
			std::string	result;
			bool		in_single_quote = false;
			bool		in_double_quote = false;
			bool		escaped = false;

			for (size_t i = 0; i < str.length(); ++i) {
				char c = str[i];

				if (escaped) { result += c; escaped = false; continue; }

				if (c == '\\') {
					if (in_single_quote)	result += c;
					else					escaped = true;
					continue;
				}

				if (c == '\'' && !in_double_quote) {
					in_single_quote = !in_single_quote;
					result += c; continue;
				}

				if (c == '"' && !in_single_quote) {
					in_double_quote = !in_double_quote;
					result += c; continue;
				}

				if (!in_single_quote && c == '$') {
					// ${VAR}
					if (i + 1 < str.length() && str[i + 1] == '{') {
						size_t start = i + 2;
						size_t end = start;
						while (end < str.length() && str[end] != '}') end++;

						if (end < str.length()) {
							std::string expanded = expand_variable(str.substr(start, end - start), env);
							result += expanded; i = end;
						} else result += c;
					}
					// $VAR
					else {
						size_t	start = i + 1;
						size_t	end = start;

						while (end < str.length() && (isalnum(static_cast<unsigned char>(str[end])) || str[end] == '_')) end++;

						if (end > start) {
							std::string var_name = str.substr(start, end - start);
							auto it = env.find(var_name);
							if (it != env.end()) result += it->second;
							else if (var_name == "NUMPROCS" || var_name == "PROCESS_NUM") result += "$" + var_name;
							i = end - 1;
						} else result += c;
					}
				} else result += c;
			}

			return (result);
		}

	#pragma endregion

#pragma endregion

#pragma region "Manage"

	#pragma region "Del"

		void ConfigParser::environment_del(std::map<std::string, std::string>& env, const std::string& key) {
			if (!key.empty()) env.erase(key);
		}

	#pragma endregion

	#pragma region "Add"

		void ConfigParser::environment_add(std::map<std::string, std::string>& env, const std::string& key, const std::string& value) {
			if (!key.empty() && !value.empty()) env[key] = value;
		}

	#pragma endregion

	#pragma region "Add Batch"

		void ConfigParser::environment_add_batch(std::map<std::string, std::string>& env, const std::string& batch) {
			if (batch.empty()) return ;

			std::vector<std::string>	pairs;
			std::string					current;
			char						quote = 0;
			bool						escaped = false;

			for (size_t i = 0; i < batch.length(); ++i) {
				char c = batch[i];

				if		(escaped)								{ current += c; escaped = false; }
				else if	(c == '\\')								  escaped = true;
				else if	(!quote && (c == '"' || c == '\''))		  quote = c;
				else if	(quote && c == quote)					  quote = 0;
				else if	(!quote && c == ',')					{ pairs.push_back(current); current.clear(); }
				else											  current += c;
			}

			if (!current.empty()) pairs.push_back(current);

			for (const std::string& pair : pairs) {
				size_t pos = pair.find('=');
				if (pos != std::string::npos) {
					std::string	key = trim(pair.substr(0, pos));
					std::string	value = trim(pair.substr(pos + 1));
					env[key] = value;
				}
			}
		}

	#pragma endregion

	#pragma region "Clone"
		
		void ConfigParser::environment_clone(std::map<std::string, std::string>& env, const std::map<std::string, std::string>& src) {
			if (env != src) env = src;
		}

	#pragma endregion

#pragma endregion

#pragma region "Initialize"

	void ConfigParser::environment_initialize() {
		environment.clear();

		for (char **env = environ; *env != nullptr; ++env) {
			std::string entry(*env);
			auto pos = entry.find('=');
			if (pos != std::string::npos) {
				std::string key = entry.substr(0, pos);
				std::string value = entry.substr(pos + 1);
				environment[key] = value;
			}
		}
	}

#pragma endregion


#pragma region "Print"

	#include <iostream>															// std::cerr

	void ConfigParser::environment_print(const std::map<std::string, std::string>& env) const {
		for (const auto& pair : env) {
			std::cerr << pair.first << "=" << pair.second << "\n";
		}
	}

#pragma endregion
