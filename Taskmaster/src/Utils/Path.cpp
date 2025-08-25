/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Path.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/24 17:00:45 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/25 23:34:45 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Utils/Utils.hpp"

	#include <pwd.h>															// getpwuid()
	#include <unistd.h>															// getuid()
	#include <filesystem>														// std::filesystem::path(), std::filesystem::canonical(), std::filesystem::temp_directory_path()

#pragma endregion

#pragma region "Expand Path"

	std::string expand_path(const std::string& path) {
		if (path.empty()) return ("");

		std::filesystem::path p;

		// is home
		if (path[0] == '~') {
			const char *home = std::getenv("HOME");
			if (!home) {
				struct passwd *pw = getpwuid(getuid());
				if (pw) home = pw->pw_dir;
			}
			if (!home) return ("");
			p = std::filesystem::path(home) / path.substr(1);
		}
		// is absolute
		else if (std::filesystem::path(path).is_absolute()) p = path;
		// is relative
		else p = std::filesystem::current_path() / path;

		// resolve symbolic links
		try { return (std::filesystem::weakly_canonical(p).string()); }
		catch (const std::filesystem::filesystem_error&) { return (""); }
	}

#pragma endregion

#pragma region "Temp Path"

	std::string temp_path() {
		try { return (std::filesystem::temp_directory_path().string()); }
		catch (const std::filesystem::filesystem_error& e) { return (""); }
	}

#pragma endregion


#pragma region "Config Path"

	std::string config_path() {
		const std::string candidates[] = {
			"../etc/supervisord.conf",
			"../supervisord.conf",
			"supervisord.conf",
			"etc/supervisord.conf",
			"/etc/supervisord.conf",
			"/etc/supervisor/supervisord.conf"
		};

		for (const auto& path : candidates) {
			std::string expanded = expand_path(path);
			if (!expanded.empty() && std::filesystem::exists(expanded)) return (expanded);
		}

		return ("");
	}

#pragma endregion

