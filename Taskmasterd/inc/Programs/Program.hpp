/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Program.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/02 17:24:36 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/04 15:53:47 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include "Programs/Process.hpp"

	#include <cstdint>															// uint16_t
	#include <string>															// std::string
	#include <vector>															// std::vector

#pragma endregion

#pragma region "Program"

	class Group;
	class Program {

		public:

			// Constructors
			Program(const std::string _name);
			Program(const Program&) = default;
			Program(Program&&) = default;
			~Program() = default;

			// Overloads
			Program& operator=(const Program&) = default;
			Program& operator=(Program&&) = default;

			// Variables
			std::string					section;
			std::string					name;
			uint16_t        			numprocs;
			uint16_t        			numprocs_start;
			bool						disabled;
			std::vector<Process>		process;
			std::vector<std::string>	groups;

		private:
		
			void	add_process();

	};

#pragma endregion
