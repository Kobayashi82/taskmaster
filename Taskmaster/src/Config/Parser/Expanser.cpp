/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Expanser.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/28 15:57:15 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/28 16:38:05 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Parser.hpp"

	#include <algorithm>

#pragma endregion

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
			if (fmt == "X") {
				std::stringstream ss;
				ss << std::hex << std::uppercase << num;
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
		} catch (...) { return (value); }

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
		// ${VAR:mod}
		size_t		colon = var_expr.find(':');
		std::string	var_name = (colon == std::string::npos) ? var_expr : var_expr.substr(0, colon);
		std::string	modifier = (colon == std::string::npos) ? "" : var_expr.substr(colon + 1);

		auto it = env.find(var_name);
		std::string value = (it != env.end()) ? it->second : "";

		if (modifier.empty()) return (value);

		// ${VAR:-default}
		if (modifier.substr(0, 1) == "-") return (value.empty() ? modifier.substr(1) : value);

		// ${VAR:=default}
		if (modifier.substr(0, 1) == "=") {
			if (value.empty()) {
				value = modifier.substr(1);
				env[var_name] = value;
			}
			return (value);
		}

		// ${VAR:+algo}
		if (modifier.substr(0, 1) == "+") return (value.empty() ? "" : modifier.substr(1));

		// Formateo con *
		if (modifier[0] == '*') return (apply_format(value, modifier));

		// Substring (números o espacios)
		if (std::isdigit(modifier[0]) || modifier[0] == ' ') return (apply_substring(value, modifier));

		return (value);
	}

#pragma endregion

#pragma region "Expand"

	std::string ConfigParser::environment_expand(std::string& str, std::map<std::string, std::string>& env) {
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
					size_t	start = i + 2;
					size_t	end = start;
					int		brace_count = 1;

					while (end < str.length() && brace_count > 0) {
						if      (str[end] == '{') brace_count++;
						else if (str[end] == '}') brace_count--;
						end++;
					}

					if (brace_count == 0) {
						std::string var_expr = str.substr(start, end - start - 1);
						std::string expanded = expand_variable(var_expr, env);
						result += expanded;
						i = end - 1;
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
						i = end - 1;
					} else result += c;
				}
			} else result += c;
		}

		return (result);
	}

#pragma endregion
