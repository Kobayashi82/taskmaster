/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Environment.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/28 12:25:58 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/28 16:31:08 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Parser.hpp"

#pragma endregion

#pragma region "Variables"

	extern char **environ;

#pragma endregion

#pragma region "Del"

	void ConfigParser::environment_del(std::map<std::string, std::string>& env, const std::string& key) {
		if (!key.empty()) env.erase(key);
	}

#pragma endregion

#pragma region "Add"

	void ConfigParser::environment_add(std::map<std::string, std::string>& env, const std::string& key, const std::string& value) {
		if (!key.empty() && !value.empty()) env[key] = value;
	}

#pragma endregion

#pragma region "Add Batch"

	void ConfigParser::environment_add_batch(std::map<std::string, std::string>& env, const std::string& batch) {
		if (batch.empty()) return ;

		std::vector<std::string>	pairs;
		std::string					current;
		char						quote = 0;
		bool						escaped = false;

		for (size_t i = 0; i < batch.length(); ++i) {
			char c = batch[i];

			if		(escaped)								{ current += c; escaped = false; }
			else if	(c == '\\')								  escaped = true;
			else if	(!quote && (c == '"' || c == '\''))		  quote = c;
			else if	(quote && c == quote)					  quote = 0;
			else if	(!quote && c == ',')					{ pairs.push_back(current); current.clear(); }
			else											  current += c;
		}

		if (!current.empty()) pairs.push_back(current);

		for (const std::string& pair : pairs) {
			size_t pos = pair.find('=');
			if (pos != std::string::npos) {
				std::string	key = trim(pair.substr(0, pos));
				std::string	value = trim(pair.substr(pos + 1));
				env[key] = value;
			}
		}
	}

#pragma endregion

#pragma region "Clone"
	
	void ConfigParser::environment_clone(std::map<std::string, std::string>& env, const std::map<std::string, std::string>& src) {
		if (env != src) env = src;
	}

#pragma endregion

#pragma region "Initialize"

	void ConfigParser::environment_initialize() {
		environment.clear();

		for (char **env = environ; *env != nullptr; ++env) {
			std::string entry(*env);
			auto pos = entry.find('=');
			if (pos != std::string::npos) {
				std::string key = entry.substr(0, pos);
				std::string value = entry.substr(pos + 1);
				environment[key] = value;
			}
		}
	}

#pragma endregion

#pragma region "Print"

	#include <iostream>															// std::cerr

	void ConfigParser::environment_print(const std::map<std::string, std::string>& env) const {
		for (const auto& pair : env) {
			std::cerr << pair.first << "=" << pair.second << "\n";
		}
	}

#pragma endregion
