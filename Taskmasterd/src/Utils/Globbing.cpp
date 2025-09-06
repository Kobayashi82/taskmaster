/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Globbing.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 14:53:31 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/06 23:14:13 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Utils/Utils.hpp"

	#include <unistd.h>															// getuid()
	#include <regex>															// std::regex_match()
	#include <filesystem>														// std::filesystem::path()
	#include <pwd.h>															// struct passwd, getpwuid()

	#include <iostream>
	#include <cstdlib>															// std::getenv()

#pragma endregion

#pragma region "Has Glob"

	bool Utils::globbing_has_glob(const std::string& pattern) {
		char	quoteChar = 0;
		bool	escaped = false;

		for (char c : pattern) {
			if (escaped)								{ escaped = false;	continue; }
			if (quoteChar != '\'' && c == '\\')			{ escaped = true;	continue; }
			if (!quoteChar && (c == '"' || c == '\''))	{ quoteChar = c;	continue; }
			if (quoteChar && c == quoteChar)			{ quoteChar = 0;	continue; }
			if (!quoteChar && (c == '*' || c == '?' || c == '['))			return (true);
		}

		return (false);
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

			if (c == '\\' && i + 1 < glob.size()) { regex += "\\"; regex += glob[++i];	continue; }

			switch (c) {
				case '*':	regex += ".*";												break;
				case '?':	regex += ".";												break;
				case '[': {	regex += "["; ++i;
					if (i < glob.size() && (glob[i] == '!' || glob[i] == '^')) { regex += "^"; ++i; }
					while (i < glob.size() && glob[i] != ']') {
						if (glob[i] == '\\' && i + 1 < glob.size()) { regex += "\\"; regex += glob[++i]; }
						else regex += glob[i];
						++i;
					}
					if (i < glob.size()) regex += "]";									break;
				}
				case '.': case '^': case '$': case '+': case '{': case '}': case '|': case '(': case ')':
					regex += "\\"; regex += c;											break;
				default:
					regex += c;															break;
			}
		}

		return (regex);
	}

#pragma endregion

#pragma region "Expand Tilde"

	std::string Utils::globbing_expand_tilde(const std::string& path) {
		if (path.empty() || path[0] != '~') return (path);

		std::filesystem::path p;

		if (path[0] == '~') {
			const char *home = std::getenv("HOME");
			if (!home) {
				struct passwd *pw = getpwuid(getuid());
				if (pw) home = pw->pw_dir;
			}
			if (!home) return ("");
			std::string suffix = (path.length() > 1 && path[1] == '/') ? path.substr(2) : path.substr(1);
			if (path.length() > 1 && path[1] == '/')	p = std::filesystem::path(home) / suffix;
			else 										p = home + path.substr(1);

			return (p);
		}

		return (path);
	}

#pragma endregion

#pragma region "Expand Glob"

	std::vector<std::string> Utils::globbing_expand_glob(const std::string& pattern) {
		std::vector<std::string>	matches, parts;
		std::filesystem::path		base, pathPattern;
		std::string					glob_pattern, processed, original, expanded_pattern;
		char						quoteChar = 0;
		bool						escaped = false;
		bool						hasGlobbing = false;

		expanded_pattern = globbing_expand_tilde(pattern);

		for (char c : expanded_pattern) {
			if (escaped)								{ escaped = false;	processed += '\\';	processed += c;	original += c;	continue; }
			if (quoteChar != '\'' && c == '\\')			{ escaped = true;														continue; }
			if (!quoteChar && (c == '"' || c == '\''))	{ quoteChar = c;														continue; }
			if (quoteChar && c == quoteChar)			{ quoteChar = 0;														continue; }

			original += c;
			
			if (quoteChar && (c == '*' || c == '?' || c == '[')) {			processed += '\\';	processed += c; }
			else {																				processed += c;
				if (!quoteChar && (c == '*' || c == '?' || c == '['))		hasGlobbing = true;
			}
		}

		if (escaped || !hasGlobbing) { matches.push_back(original); return (matches); }
		pathPattern = std::filesystem::path(processed);

		for (const auto& part : pathPattern)
			parts.push_back(part.string());

		size_t glob_idx = 0;
		for (; glob_idx < parts.size(); ++glob_idx)
			if (parts[glob_idx].find_first_of("*?[") != std::string::npos) break;

		for (size_t i = 0; i < glob_idx; ++i)
			base /= parts[i];

		glob_pattern = std::filesystem::path("").string();
		for (size_t i = glob_idx; i < parts.size(); ++i) {
			if (i > glob_idx) glob_pattern += "/";
			glob_pattern += parts[i];
		}

		if (base.empty()) base = ".";

		try {
			std::error_code ec;
			auto it = std::filesystem::directory_iterator(base, ec);
			if (ec) { matches.push_back(original); return (matches); }

			for (const auto& entry : it) {
				std::error_code entry_ec;
				auto entry_path = entry.path();
				auto filename = entry_path.filename().string();

				if (!filename.empty() && filename[0] == '.') continue;

				std::filesystem::path rel = std::filesystem::relative(entry_path, base, entry_ec);
				if (!entry_ec && globbing_match_glob(glob_pattern, rel.string())) {
					if (base == "." && original.substr(0, 2) == "./")						matches.push_back("./" + filename);
					else if (base == "." && glob_pattern.find('/') == std::string::npos)	matches.push_back(filename);
					else																	matches.push_back(entry_path.string());
				}
			}
			std::sort(matches.begin(), matches.end());
		} catch (const std::filesystem::filesystem_error&) { matches.push_back(original); }

		if (matches.empty()) matches.push_back(original);

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
			} else {
				std::string clean_pattern = remove_quotes(pattern);
				result.push_back(globbing_expand_tilde(clean_pattern));
			}
		}

		return (result);
	}

#pragma endregion
