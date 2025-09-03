/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   String.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/03 22:50:20 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/03 22:52:44 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Utils/Utils.hpp"

	#include <algorithm>														// std::transform()

#pragma endregion

#pragma region "Trim"

	std::string Utils::trim(const std::string& str) {
		const auto first = str.find_first_not_of(" \f\v\t\r\n");
		if (first == std::string::npos) return {};

		const auto last = str.find_last_not_of(" \f\v\t\r\n");
		return (str.substr(first, (last - first) + 1));
	}

	std::string Utils::ltrim(const std::string& str) {
		const auto first = str.find_first_not_of(" \f\v\t\r\n");
		if (first == std::string::npos) return {};

		return (str.substr(first));
	}

	std::string Utils::rtrim(const std::string& str) {
		const auto last = str.find_last_not_of(" \f\v\t\r\n");
		if (last == std::string::npos) return {};

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
