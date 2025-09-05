/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   String.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/03 22:50:20 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/05 13:42:10 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Utils/Utils.hpp"

	#include <algorithm>														// std::transform()
	#include <stdexcept>														// std::runtime_error()

#pragma endregion

#pragma region "Trim"

	std::string Utils::trim(const std::string& str) {
		const auto first = str.find_first_not_of(" \f\v\t\r\n");
		if (first == std::string::npos) return {};

		bool escaped = false;
		char quoteChar = 0;
		size_t last = first;

		for (size_t i = 0; i < str.length(); ++i) {
			char c = str[i];

			if (escaped)								{ escaped = false;	last = i;	continue; }
			if (!quoteChar && c == '\\')				{ escaped = true;	last = i;	continue; }
			if (!quoteChar && (c == '"' || c == '\''))	{ quoteChar = c;	last = i;	continue; }
			if (quoteChar && c == quoteChar)			{ quoteChar = 0;	last = i;	continue; }
			if (!std::isspace(c))						{ last = i; }
		}

		if (last < first) last = first;
		return (str.substr(first, (last - first) + 1));
	}

	std::string Utils::ltrim(const std::string& str) {
		const auto first = str.find_first_not_of(" \f\v\t\r\n");
		if (first == std::string::npos) return {};

		return (str.substr(first));
	}

	std::string Utils::rtrim(const std::string& str) {
		if (str.find_last_not_of(" \f\v\t\r\n") == std::string::npos) return {};

		bool escaped = false;
		char quoteChar = 0;
		size_t last = 0;

		for (size_t i = 0; i < str.length(); ++i) {
			char c = str[i];

			if (escaped)								{ escaped = false;	last = i;	continue; }
			if (!quoteChar && c == '\\')				{ escaped = true;	last = i;	continue; }
			if (!quoteChar && (c == '"' || c == '\''))	{ quoteChar = c;	last = i;	continue; }
			if (quoteChar && c == quoteChar)			{ quoteChar = 0;	last = i;	continue; }
			if (!std::isspace(c))						{ last = i; }
		}

		return (str.substr(0, last + 1));
	}

#pragma endregion

#pragma region "To Upper"

	std::string Utils::toUpper(const std::string& str) {
		std::string result = str;
		std::transform(result.begin(), result.end(), result.begin(), ::toupper);

		return (result);
	}

#pragma endregion

#pragma region "To Lower"

	std::string Utils::toLower(const std::string& str) {
		std::string result = str;
		std::transform(result.begin(), result.end(), result.begin(), ::tolower);

		return (result);
	}

#pragma endregion

#pragma region "Is Digit"

	bool Utils::isDigit(const std::string& str) {
		return (!str.empty() && std::all_of(str.begin(), str.end(), [](unsigned char c) { return (std::isdigit(c)); }));
	}

#pragma endregion
