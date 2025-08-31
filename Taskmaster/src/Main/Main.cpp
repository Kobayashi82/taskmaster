/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/11 19:29:12 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/31 13:43:33 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Options.hpp"
	#include "Config/Parser.hpp"

	#include <iostream>															// std::cerr()

#pragma endregion

#pragma region "Load Configuration"

	static int load_configuration(int argc, char **argv) {
		int result = 0;

		try {
			ConfigOptions Options;
			if ((result = Options.parse(argc, argv))) return (result);
			Parser.parse(Options.configuration);
			Parser.merge_options(Options);
			std::cout << "Configuration loaded succesfully\n";
		} catch (const std::exception& e) { std::cerr << e.what(); return (2); }

		return (0);
	}

#pragma endregion

#pragma region "Main"

	int main(int argc, char **argv) {
		int result = 0;
		if ((result = load_configuration(argc, argv))) return (result - 1);

		return (result);
	}

#pragma endregion
