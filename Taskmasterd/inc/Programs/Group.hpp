/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Group.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/02 17:24:36 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/09 12:45:41 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include <string>															// std::string
	#include <cstdint>															// uint16_t
	#include <map>																// std::map
	#include <vector>															// std::vector

#pragma endregion

#pragma region "Group"

	class Program;
	class Group {

		public:

			// Constructors
			Group() = delete;
			Group(const std::string _name);
			Group(const Group&) = default;
			Group(Group&&) = default;
			~Group() = default;

			// Overloads
			Group& operator=(const Group&) = default;
			Group& operator=(Group&&) = default;

			// Variables
			std::string					section;
			std::string					name;
			uint16_t					priority;
			std::vector<std::string>	programs;

		private:

			std::string	expand_vars(std::map<std::string, std::string>& env, const std::string& key);
			void		initialize();

	};

#pragma endregion
