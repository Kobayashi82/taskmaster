/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Array.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/05 10:17:15 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/05 10:25:14 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Utils/Utils.hpp"

	#include <cstring>															// strdup()

#pragma endregion

	char** Utils::toArray(const std::initializer_list<std::string>& src) {
		char **array = new char*[src.size() + 1];

		size_t i = 0;
		for (auto& v : src) array[i++] = strdup(v.c_str());
		array[i] = nullptr;

		return (array);
	}

	char** Utils::toArray(const std::map<std::string, std::string>& src) {
		char **array = new char *[src.size() + 1];

		size_t i = 0;
		for (auto& kv : src) array[i++] = strdup(std::string(kv.first + "=" + kv.second).c_str());
		array[i] = nullptr;

		return (array);
	}

	char** Utils::toArray(const std::vector<std::string>& src) {
		char **array = new char *[src.size() + 1];

		size_t i = 0;
		for (auto& v : src) array[i++] = strdup(v.c_str());
		array[i] = nullptr;

		return (array);
	}

	void Utils::array_free(char **array) {
		if (array) {
			for (size_t i = 0; array[i]; ++i) free(array[i]);
			delete[] array;
		}
	}

