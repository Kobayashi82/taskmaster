/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Array.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/05 10:17:15 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/08 17:40:25 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Utils/Utils.hpp"

	#include <cstring>															// strdup()

#pragma endregion

#pragma region "Array from String"

	char** Utils::toArray(const std::string& src) {
		static std::string split = " \f\v\t\r\n";

		std::vector<std::string>	vec_array;
		std::string					current;
		char						quoteChar = 0;
		bool						escaped = false;

		auto pushToken = [&]() {
			if (!current.empty()) {
				std::string token = Utils::trim(current);
				if (!token.empty()) vec_array.push_back(token);
				current.clear();
			}
		};

		for (char c : src) {
			if (escaped)											{ escaped = false;	current += c;	continue; }
			if (quoteChar != '\'' && c == '\\')						{ escaped = true;					continue; }
			if (!quoteChar && (c == '"' || c == '\''))				{ quoteChar = c; 					continue; }
			if (quoteChar && c == quoteChar)						{ quoteChar = 0;					continue; }
			if (!quoteChar && split.find(c) != std::string::npos)	{ pushToken();						continue; }

			current += c;
		}
		pushToken();

		char **array = new char *[vec_array.size() + 1];

		size_t i = 0;
		for (auto& v : vec_array) array[i++] = strdup(v.c_str());
		array[i] = nullptr;

		return (array);
	}

#pragma endregion

#pragma region "Array from List"

	char** Utils::toArray(const std::initializer_list<std::string>& src) {
		char **array = new char*[src.size() + 1];

		size_t i = 0;
		for (auto& v : src) array[i++] = strdup(v.c_str());
		array[i] = nullptr;

		return (array);
	}

#pragma endregion

#pragma region "Array from Map"

	char** Utils::toArray(const std::map<std::string, std::string>& src) {
		char **array = new char *[src.size() + 1];

		size_t i = 0;
		for (auto& kv : src) array[i++] = strdup(std::string(kv.first + "=" + kv.second).c_str());
		array[i] = nullptr;

		return (array);
	}

#pragma endregion

#pragma region "Array from Vector"

	char** Utils::toArray(const std::vector<std::string>& src) {
		char **array = new char *[src.size() + 1];

		size_t i = 0;
		for (auto& v : src) array[i++] = strdup(v.c_str());
		array[i] = nullptr;

		return (array);
	}

#pragma endregion

#pragma region "Free"

	void Utils::array_free(char **array) {
		if (array) {
			for (size_t i = 0; array[i]; ++i) free(array[i]);
			delete[] array;
		}
	}

#pragma endregion
