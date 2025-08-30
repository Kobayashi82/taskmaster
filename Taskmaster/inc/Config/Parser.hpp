/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/24 21:47:27 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/30 16:16:46 by vzurera-         ###   ########.fr       */
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
			std::map<std::string, std::string>							environment_config;
			std::filesystem::path										configPath;
			bool														in_include;
			bool														section_on_error;

			// Initialize
			void						initialize();
			void						default_values();

			// Section
			bool						is_section(const std::string& line) const;
			std::string					section_type(const std::string& section) const;
			std::string					section_extract(const std::string& line) const;
			void						section_parse(const std::string& line);

			// Keys
			std::string					key_remove_comments(const std::string& line) const;
			bool						key_valid(const std::string& section, const std::string& key) const;
			void						key_parse(const std::string& line);

			// Globbing
			bool						globbing_has_glob(const std::string& path);
			bool						globbing_match_glob(const std::string& pattern, const std::string& text);
			std::string					globbing_glob_to_regex(const std::string& glob);
			std::vector<std::string>	globbing_expand_glob(const std::string& pattern);
			std::vector<std::string>	globbing_expand(const std::vector<std::string>& patterns);

			// Include
			void						include_parse(const std::string& filePath);
			void						include_process();

			// Environment
			std::string					environment_apply_format(const std::string& value, const std::string& format) const;
			std::string					environment_apply_substring(const std::string& value, const std::string& substr_spec) const;
			std::string					environment_expand_expr(std::map<std::string, std::string>& env, const std::string& var_expr) const;
			std::string					environment_expand(std::map<std::string, std::string>& env, const std::string& str, bool split_comma = false) const;
			bool						environment_validate(const std::string& env_string) const;
			void						environment_del(std::map<std::string, std::string>& env, const std::string& key);
			void						environment_add(std::map<std::string, std::string>& env, const std::string& key, const std::string& value);
			void						environment_add(std::map<std::string, std::string>& env, const std::map<std::string, std::string>& src, bool overwrite = false);
			void						environment_add_batch(std::map<std::string, std::string>& env, const std::string& batch);
			void						environment_clone(std::map<std::string, std::string>& env, const std::map<std::string, std::string>& src);
			void						environment_initialize(std::map<std::string, std::string>& env);
			void						environment_print(const std::map<std::string, std::string>& env) const;

			// Validation
			bool						valid_bool(const std::string& value) const;
			bool						valid_number(const std::string& value, long min = 0, long max = 1024 * 1024 * 1024) const;
			bool						valid_path(const std::string& value, bool is_directory = false, bool allow_auto = false, bool allow_none = false, bool allow_syslog = false) const;
			bool						valid_signal(const std::string& value) const;
			bool						valid_code(const std::string& value) const;
			bool						valid_loglevel(const std::string& value) const;
			bool						valid_autorestart(const std::string& value) const;
			bool						valid_umask(const std::string& value) const;
			bool						valid_user(const std::string& value) const;
			bool						valid_chown(const std::string& value) const;
			bool						valid_password(const std::string& value) const;
			bool						valid_port(const std::string& value) const;
			bool						valid_serverurl(const std::string &value) const;
			void						validate_taskmasterd(const std::string& section, const std::string& key, std::string& value) const;
			void						validate_program(const std::string& section, std::string& key, std::string& value) const;
			void						validate_unix_server(const std::string& section, std::string& key, std::string& value) const;
			void						validate_inet_server(const std::string& section, std::string& key, std::string& value) const;
			void						validate_group(const std::string& section, std::string& key, std::string& value) const;
			void						validate(const std::string& section, std::string& key, std::string& value) const;

			// Utils
			std::string					trim(const std::string& str) const;
			std::string					toLower(const std::string& str) const;
			std::string					toUpper(const std::string& str) const;

			std::string					extract_command(const std::string& line) const;
			bool						is_exec(const std::string& path) const;
			bool						command_is_executable(const std::string& input, std::string& resolved) const;

		public:

			// Constructors
			ConfigParser();
			ConfigParser(const ConfigParser&) = delete;
			~ConfigParser() = default;

			// Overloads
			ConfigParser& operator=(const ConfigParser&) = delete;

			// Parser
			void								parse(const std::string& filePath = "");
			void								merge_options(ConfigOptions& Options);
			void								print() const;

			// Getters
			std::string							get_value(const std::string& section, const std::string& key, const std::string& defaultValue = "") const;
			bool								has_section(const std::string& section) const;
			std::map<std::string, std::string>	get_section(const std::string& section, bool use_defaults = false) const;
			std::vector<std::string>			get_program() const;
			std::vector<std::string>			get_group() const;

			// Utils
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
