/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/11 19:29:12 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/01 13:47:39 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Options.hpp"
	#include "Config/Parser.hpp"
	#include "Logging/TaskmasterLog.hpp"

	#include <iostream>															// std::cerr()

#pragma endregion

#pragma region "Load Configuration"

	static int load_configuration(int argc, char **argv) {
		int result = 0;

		ConfigOptions Options;
		Log.info("Iniciado carga");
		if ((result = Options.parse(argc, argv)))	return (result);
		if (Parser.parse(Options.configuration))	return (2);
		Parser.merge_options(Options);
		Parser.print();

		Log.info("Mesaje de prueba");
		Log.error("Mesaje de error");
		Log.generic("Mensaje generico");
		Log.debug("Mesaje de debug");
		Log.set_logfile_stdout(true);
		Log.set_logfile(Parser.get_value("taskmasterd", "logfile"));
		Log.set_logfile_ready(true);
		Log.generic("Configuration loaded succesfully");
		Log.info("cerrando");
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
