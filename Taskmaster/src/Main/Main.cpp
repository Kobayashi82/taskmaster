/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/11 19:29:12 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/27 12:41:27 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Options.hpp"
	#include "Config/Parser.hpp"

	#include <iostream>															// std::cerr()
	#include <unistd.h>															// getuid()
	#include <fcntl.h>															// open()
	#include <sys/file.h>														// flock()
	#include <filesystem>														// path(), absolute()

#pragma endregion

#pragma region "Resolve Path"

	std::string resolve_path(const std::string &cmd) {
		if (cmd.find('/') != std::string::npos) return (cmd);

		const char *path_env = std::getenv("PATH");
		if (!path_env) return (cmd);

		std::stringstream ss(path_env);
		std::string dir;
		while (std::getline(ss, dir, ':')) {
			std::filesystem::path candidate = std::filesystem::path(dir) / cmd;
			if (::access(candidate.c_str(), X_OK) == 0) return (candidate.string());
		}

		return (cmd);
	}

#pragma endregion

#pragma region "Validate Input"

	static int load_configuration(int argc, char **argv) {
		int result = 0;

		try {
			ConfigOptions Options;
			if ((result = Options.parse(argc, argv))) return (result);
			// Options::validate();
			Parser.parse_file("taskmasterd.ini");
			Parser.merge_options(Options);
			Parser.print();
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

// pkill -u $USER supervisord; supervisord -c ~/supervisord.conf

// pkill -u $USER taskmasterd; taskmasterd -c ~/taskmasterd.conf

// ./taskmasterd.conf					- En el directorio actual
// ./etc/taskmasterd.conf				- En subdirectorio etc/ del directorio actual
// /etc/taskmasterd.conf				- En el directorio global /etc/
// /etc/taskmaster/taskmasterd.conf		- En el directorio global /etc/taskmasterd

// Advertencia de seguridad:
//	A warning is emitted when taskmaster is started as root without '-c' argument,
//	porque alguien podría engañarte para ejecutartaskmaster desde un directorio que
//	contiene un archivo taskmasterd.conf malicioso
