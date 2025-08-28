/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/24 21:47:27 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/28 16:38:16 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include "Config/Options.hpp"

	#include <cstdint>															// uint16_t
	#include <set>																// std::set
	#include <map>																// std::map
	#include <vector>															// std::vector
	#include <filesystem>														// std::filesystem::path()

#pragma endregion

#pragma region "ConfigParser"

	class ConfigParser {

		private:

			// Variables
			std::string													currentSection;
			std::map<std::string, std::map<std::string, std::string>>	sections;
			std::set<std::string>										validSections;
			std::map<std::string, std::set<std::string>>				validKeys;
			std::map<std::string, std::map<std::string, std::string>>	defaultValues;

			std::map<std::string, std::string>							environment;
			std::filesystem::path										configPath;
			bool														in_include;
			bool														section_on_error;


			// Initialize
			void		initialize();
			void		default_values();

			// Keys
			bool		valid_key(const std::string& section, const std::string& key) const;
			void		parse_key(const std::string& line);

			// Section
			std::string	section_type(const std::string& section) const;
			bool		valid_section(const std::string& section) const;
			bool		is_section(const std::string& line) const;
			std::string	extract_section(const std::string& line) const;
			void		parse_section(const std::string& line);

			// Validation
			bool		valid_bool(const std::string& value) const;
			bool		valid_number(const std::string& value, long min = 0, long max = 2147483647) const;
			bool		valid_path(const std::string& value, bool is_directory) const;
			bool		valid_signal(const std::string& value) const;
			bool		valid_code(const std::string& value) const;
			bool		valid_loglevel(const std::string& value) const;
			bool		valid_autorestart(const std::string& value) const;
			bool		valid_umask(const std::string& value) const;
			bool		valid_user(const std::string& value) const;
			void		validate_taskmasterd(const std::string& section, const std::string& key, std::string& value) const;
			void		validate_program(const std::string& section, std::string& key, std::string& value) const;
			void		validate_unix_server(const std::string& section, std::string& key, std::string& value) const;
			void		validate_inet_server(const std::string& section, std::string& key, std::string& value) const;
			void		validate_group(const std::string& section, std::string& key, std::string& value) const;
			void		validate(const std::string& section, std::string& key, std::string& value) const;

			// Include
			void		parse_include_file(const std::string& filePath);
			void		process_includes();

			// Expand
			std::string	apply_format(const std::string& value, const std::string& format);
			std::string	apply_substring(const std::string& value, const std::string& substr_spec);
			std::string	expand_variable(const std::string& var_expr, std::map<std::string, std::string>& env);
			std::string	environment_expand(std::string& str, std::map<std::string, std::string>& env);


			// Environment
			void	environment_del(std::map<std::string, std::string>& env, const std::string& key);
			void	environment_add(std::map<std::string, std::string>& env, const std::string& key, const std::string& value);
			void	environment_add_batch(std::map<std::string, std::string>& env, const std::string& batch);
			void	environment_clone(std::map<std::string, std::string>& env, const std::map<std::string, std::string>& src);
			void	environment_initialize();
			void	environment_print(const std::map<std::string, std::string>& env) const;

		public:

			// Constructors
			ConfigParser();
			ConfigParser(const ConfigParser&) = delete;
			~ConfigParser() = default;

			// Overloads
			ConfigParser& operator=(const ConfigParser&) = delete;

			// Parser
			void	parse_file(const std::string& filePath);
			void	merge_options(ConfigOptions& Options);
			void	print() const;

			// Getters
			std::string							get_value(const std::string& section, const std::string& key, const std::string& defaultValue = "") const;
			bool								has_section(const std::string& section) const;
			std::map<std::string, std::string>	get_section(const std::string& section) const;
			std::map<std::string, std::string>	get_section_with_defaults(const std::string& section) const;
			std::vector<std::string>			get_program() const;
			std::vector<std::string>			get_group() const;

			// Globbing
			bool								has_glob(const std::string& path);
			bool								match_glob(const std::string& pattern, const std::string& text);
			std::string							glob_to_regex(const std::string& glob);
			std::vector<std::string>			expand_glob(const std::string& pattern);
			std::vector<std::string>			expand_globs(const std::vector<std::string>& patterns);

			// Utils
			std::string							trim(const std::string& str) const;
			std::string							toLower(const std::string& str) const;
			std::string							toUpper(const std::string& str) const;
			std::string							expand_path(const std::string& path, const std::string current_path = "", bool expand_symbolic = true) const;
			std::string							temp_path() const;
			std::string							config_path() const;
			std::vector<std::string>			parse_files(const std::string& fileString);
			int									check_fd_limit(uint16_t minfds) const;
			int									check_process_limit(uint16_t minprocs) const;
			long								parse_size(const std::string &value) const;

	};

#pragma endregion
	
#pragma region "Variables"

	extern ConfigParser Parser;

#pragma endregion
