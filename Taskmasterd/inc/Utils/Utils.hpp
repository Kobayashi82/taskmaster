/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/03 22:47:54 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/04 13:24:41 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include <string>															// std::string
	#include <climits>															// LONG_MIN, LONG_MAX
	#include <cstdint>															// uint8_t
	#include <map>																// std::map
	#include <vector>															// std::vector

#pragma endregion

#pragma region "Enumarators"

	enum e_level { DEBUG, INFO, WARNING, ERROR, CRITICAL, GENERIC };

#pragma endregion

#pragma region "Utils"

	class Utils {

		private:

			// Environment
			static std::string	environment_apply_format(const std::string& value, const std::string& format);
			static std::string	environment_apply_substring(const std::string& value, const std::string& substr_spec);
			static std::string	environment_expand_expr(std::map<std::string, std::string>& env, const std::string& var_expr);

		public:

			// Structures
			struct ErrorInfo {
				std::string	filename;
				std::string	msg;
				uint8_t		level;
				uint16_t	line;
				uint16_t	order;
			};

			// Variables
			static std::vector<ErrorInfo>				errors;
			static uint16_t								errors_maxLevel;
			static std::map<std::string, std::string>	environment;

			// Constructors
			Utils() = delete;
			Utils(const Utils&) = delete;
			Utils(Utils&&) = delete;
			~Utils() = delete;

			// Overloads
			Utils& operator=(const Utils&) = delete;
			Utils& operator=(Utils&&) = delete;

			// String
			static std::string	trim(const std::string& str);
			static std::string	ltrim(const std::string& str);
			static std::string	rtrim(const std::string& str);
			static std::string	toLower(const std::string& str);
			static std::string	toUpper(const std::string& str);
			static bool			isDigit(const std::string& str);

			// Path
			static std::string	expand_path(const std::string& path, const std::string current_path = "", bool expand_symbolic = true, bool weakly = true);
			static std::string	temp_path();

			// Globbing
			static bool						globbing_has_glob(const std::string& path);
			static bool						globbing_match_glob(const std::string& pattern, const std::string& text);
			static std::string				globbing_glob_to_regex(const std::string& glob);
			static std::vector<std::string>	globbing_expand_glob(const std::string& pattern);
			static std::vector<std::string>	globbing_expand(const std::vector<std::string>& patterns);

			// Environment
			static std::string	environment_expand(std::map<std::string, std::string>& env, const std::string& str, std::string split = "");
			static bool			environment_validate(const std::string& env_string);
			static void			environment_del(std::map<std::string, std::string>& env, const std::string& key);
			static void			environment_add(std::map<std::string, std::string>& env, const std::string& key, const std::string& value);
			static void			environment_add(std::map<std::string, std::string>& env, const std::map<std::string, std::string>& src, bool overwrite = false);
			static void			environment_add_batch(std::map<std::string, std::string>& env, const std::string& batch);
			static std::string	environment_get(const std::map<std::string, std::string>& env, const std::string& key);
			static void			environment_clone(std::map<std::string, std::string>& env, const std::map<std::string, std::string>& src);
			static void			environment_initialize(std::map<std::string, std::string>& env);
			static void			environment_print(const std::map<std::string, std::string>& env);

			// Parser
			static std::string	remove_comments(const std::string& line);
			static int			parse_fd_limit(uint16_t minfds);
			static int			parse_process_limit(uint16_t minprocs);
			static long			parse_size(const std::string &value);
			static long			parse_number(const std::string& value, long min = LONG_MIN, long max = LONG_MAX, long default_value = 0);
			static uint8_t		parse_signal(const std::string& value);
			static int			parse_bool(const std::string &value, bool unexpected = false);
			static uint8_t		parse_loglevel(const std::string &value);
			static std::string 	parse_executable(const std::string& value);

			// Error
			static void			error_add(std::string& filename, std::string msg, uint8_t level, uint16_t line, uint16_t order);
			static void			error_print();

	};

#pragma endregion
