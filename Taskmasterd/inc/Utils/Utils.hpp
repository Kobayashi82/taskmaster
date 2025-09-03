/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/03 22:47:54 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/03 23:40:54 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include <string>															// std::string

#pragma endregion

#pragma region "Utils"

	class Utils {

		public:

			// String
			static std::string	trim(const std::string& str);
			static std::string	ltrim(const std::string& str);
			static std::string	rtrim(const std::string& str);
			static std::string	toLower(const std::string& str);
			static std::string	toUpper(const std::string& str);
			static bool			isDigit(const std::string& str);

			// Path
			static std::string	expand_path(const std::string& path, const std::string current_path = "", bool expand_symbolic = true, bool weakly = true);
			static std::string	temp_path();

	};

#pragma endregion
