/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Manager.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/02 17:24:36 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/04 15:53:29 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include "Programs/Program.hpp"
	#include "Programs/Group.hpp"

	#include <vector>															// std::vector

#pragma endregion

#pragma region "ProgramManager"

	class ProgramManager {

		public:

			// Constructors
			ProgramManager() = default;
			ProgramManager(const ProgramManager&) = delete;
			~ProgramManager() = default;

			// Overloads
			ProgramManager& operator=(const ProgramManager&) = delete;

			// Variables
			std::vector<Program>	Programs;
			std::vector<Group>		Groups;

			void	initialize();

		private:

	};

#pragma endregion
	
#pragma region "Variables"

	extern ProgramManager Manager;

#pragma endregion
