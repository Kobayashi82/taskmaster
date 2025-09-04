/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/11 19:29:12 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/04 13:50:39 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Options.hpp"
	#include "Config/Config.hpp"
	#include "Programs/Manager.hpp"
	#include "Logging/TaskmasterLog.hpp"

	#include <iostream>															// std::cerr()

#pragma endregion

#pragma region "Main"

	int main(int argc, char **argv) {
		int result = 0;

		if ((result = Config.load(argc, argv))) return (result) - 1;
		std::cout << Manager.Programs[1].process[0].command << "\n";
		std::cout << Manager.Programs[0].groups[0] << "\n";

		Log.info("cerrando");

		return (result);
	}

#pragma endregion
