/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Path.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/03 23:00:37 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/03 23:10:42 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Utils/Utils.hpp"

	#include <unistd.h>															// getuid()
	#include <filesystem>														// std::filesystem::path(), std::filesystem::parent_path(), std::filesystem::current_path(), std::filesystem::temp_directory_path(), std::filesystem::weakly_canonical(), std::filesystem::exists()
	#include <pwd.h>															// struct passwd, getpwnam()

#pragma endregion

#pragma region "Expand Path"

	std::string Utils::expand_path(const std::string& path, const std::string current_path, bool expand_symbolic, bool weakly) {
		if (path.empty()) return ("");

		std::filesystem::path p;
		std::filesystem::path cp = (current_path.empty()) ? std::filesystem::current_path() : std::filesystem::path(current_path);

		// is home
		if (path[0] == '~') {
			const char *home = std::getenv("HOME");
			if (!home) {
				struct passwd *pw = getpwuid(getuid());
				if (pw) home = pw->pw_dir;
			}
			if (!home) return ("");
			std::string suffix = (path.length() > 1 && path[1] == '/') ? path.substr(2) : path.substr(1);
    		p = std::filesystem::path(home) / suffix;
		}
		// is absolute
		else if (std::filesystem::path(path).is_absolute()) p = path;
		// is relative
		else p = cp / path;

		// resolve symbolic links
		if (expand_symbolic) {
			if (weakly) {
				try { return (std::filesystem::weakly_canonical(p).string()); }
				catch (const std::filesystem::filesystem_error&) { return (""); }
			} else {
				try { return (std::filesystem::canonical(p).string()); }
				catch (const std::filesystem::filesystem_error&) { return (""); }
			}
		}

		return (p);
	}

#pragma endregion

#pragma region "Temp Path"

	std::string Utils::temp_path() {
		try { return (std::filesystem::temp_directory_path().string()); }
		catch (const std::filesystem::filesystem_error& e) { return (""); }
	}

#pragma endregion
