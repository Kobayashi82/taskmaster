/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Group.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/02 17:24:36 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/03 19:39:59 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include <cstdint>															// uint16_t
	#include <string>															// std::string
	#include <vector>															// std::vector

#pragma endregion

#pragma region "Group"

	class Program;
	class Group {

		public:

			// Variables
			std::string		section;
			std::string		name;

			std::vector<Program *> programs;

			// Constructors
			Group(const std::string _name);
			Group(const Group&) = default;
			~Group() = default;

			// Overloads
			Group& operator=(const Group&) = default;

		private:

	};

#pragma endregion
