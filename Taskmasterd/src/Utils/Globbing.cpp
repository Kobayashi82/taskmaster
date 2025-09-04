/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Globbing.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 14:53:31 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/04 11:57:45 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Utils/Utils.hpp"

	#include <regex>															// std::regex_match()
	#include <filesystem>														// std::filesystem::path()

#pragma endregion

#pragma region "Has Glob"

	bool Utils::globbing_has_glob(const std::string& path) {
		return (path.find_first_of("*?[") != std::string::npos);
	}

#pragma endregion

#pragma region "Match Glob"

	bool Utils::globbing_match_glob(const std::string& pattern, const std::string& text) {
		try {
			std::string regex_pattern = globbing_glob_to_regex(pattern);
			std::regex r(regex_pattern, std::regex_constants::nosubs);

			return (std::regex_match(text, r));
		} catch (const std::regex_error&) { return {}; }
	}

#pragma endregion

#pragma region "Glob to Regex"

	std::string Utils::globbing_glob_to_regex(const std::string& glob) {
		std::string regex;
		regex.reserve(glob.size() * 2);

		for (size_t i = 0; i < glob.size(); ++i) {
			char c = glob[i];

			switch (c) {
				case '*':	regex += ".*";				break;
				case '?':	regex += ".";				break;
				case '[': {	regex += "["; ++i;
					if (i < glob.size() && (glob[i] == '!' || glob[i] == '^')) { regex += "^"; ++i; }
					while (i < glob.size() && glob[i] != ']') {
						if (glob[i] == '\\' && i + 1 < glob.size()) { regex += "\\"; regex += glob[++i]; }
						else regex += glob[i];
						++i;
					}
					if (i < glob.size()) regex += "]";	break;
				}
				case '.': case '^': case '$': case '+': case '{': case '}': case '|': case '(': case ')': case '\\':
					regex += "\\"; regex += c;			break;
				default:
					regex += c;							break;
			}
		}

		return (regex);
	}

#pragma endregion

#pragma region "Expand Glob"

	std::vector<std::string> Utils::globbing_expand_glob(const std::string& pattern) {
		std::vector<std::string>	matches, parts;
		std::filesystem::path		base, pathPattern(pattern);
		std::string					glob_pattern;

		for (const auto& part : pathPattern) {
			parts.push_back(part.string());
		}

		size_t glob_idx = 0;
		for (; glob_idx < parts.size(); ++glob_idx) {
			if (globbing_has_glob(parts[glob_idx])) break;
		}

		for (size_t i = 0; i < glob_idx; ++i) base /= parts[i];

		glob_pattern = std::filesystem::path("").string();
		for (size_t i = glob_idx; i < parts.size(); ++i) {
			if (i > glob_idx) glob_pattern += "/";
			glob_pattern += parts[i];
		}
		if (base.empty()) base = ".";

		try {
			for (const auto& entry : std::filesystem::recursive_directory_iterator(base)) {
				if (entry.is_regular_file()) {
					std::filesystem::path rel = std::filesystem::relative(entry.path(), base);
					if (globbing_match_glob(glob_pattern, rel.string())) matches.push_back(entry.path().string());
				}
			}
			std::sort(matches.begin(), matches.end());
		} catch (const std::filesystem::filesystem_error&) { matches.push_back(pattern); }

		if (matches.empty()) matches.push_back(pattern);

		return (matches);
	}

#pragma endregion

#pragma region "Expand Globs"

	std::vector<std::string> Utils::globbing_expand(const std::vector<std::string>& patterns) {
		std::vector<std::string> result;

		for (const auto& pattern : patterns) {
			if (globbing_has_glob(pattern)) {
				auto matches = globbing_expand_glob(pattern);
				result.insert(result.end(), matches.begin(), matches.end());
			} else result.push_back(pattern);
		}

		return (result);
	}

#pragma endregion
