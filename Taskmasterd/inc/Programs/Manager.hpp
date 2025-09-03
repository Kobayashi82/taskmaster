/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Manager.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/02 17:24:36 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/03 13:48:10 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include "Programs/Program.hpp"

	#include <vector>															// std::vector

#pragma endregion

#pragma region "ProgramManager"

	class ProgramManager {

		public:

			// Variables
			std::vector<Program>	Programs;

			// Constructors
			ProgramManager();
			ProgramManager(const ProgramManager&) = delete;
			~ProgramManager() = default;

			// Overloads
			ProgramManager& operator=(const ProgramManager&) = delete;

		private:

	};

#pragma endregion
	
#pragma region "Variables"

	extern ProgramManager Manager;

#pragma endregion
