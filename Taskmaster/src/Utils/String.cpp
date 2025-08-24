/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   String.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/24 19:36:01 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/24 22:09:48 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils/Utils.hpp"

#include <string>
#include <algorithm>

	std::string trim(const std::string& str) {
		size_t first = str.find_first_not_of(" \t\r\n");
		if (first == std::string::npos) return ("");

		size_t last = str.find_last_not_of(" \t\r\n");
		return (str.substr(first, (last - first + 1)));
	}

	std::string toLower(const std::string& str) {
		std::string result = str;
		std::transform(result.begin(), result.end(), result.begin(), ::tolower);

		return result;
	}
