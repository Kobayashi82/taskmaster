/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/24 21:47:27 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/02 20:56:40 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include "Config/Options.hpp"

	#include <cstdint>															// uint16_t
	#include <set>																// std::set
	#include <map>																// std::map
	#include <vector>															// std::vector

#pragma endregion

#pragma region "ConfigParser"

	class ConfigParser {

		public:

			// Structures
			typedef struct {
				std::string	value;
				std::string	filename;
				uint16_t	line;
				uint16_t	order;
			} ConfigEntry;
			typedef struct {
				std::string	filename;
				std::string	msg;
				uint8_t		level;
				uint16_t	line;
				uint16_t	order;
			}	ErrorInfo;

			// Variables
			bool	is_root;

			// Constructors
			ConfigParser();
			ConfigParser(const ConfigParser&) = delete;
			~ConfigParser() = default;

			// Overloads
			ConfigParser& operator=(const ConfigParser&) = delete;

			// Parser
			int									load(int argc, char **argv);
			void								print() const;

			// Validation
			int									validate_options(ConfigOptions& Options) const;

			// Getters
			ConfigEntry*						get_value_entry(const std::string& section, const std::string& key);
			std::string							get_value(const std::string& section, const std::string& key) const;
			bool								has_section(const std::string& section) const;
			std::map<std::string, ConfigEntry>	get_section(const std::string& section, bool use_defaults = false) const;
			std::vector<std::string>			get_program() const;
			std::vector<std::string>			get_group() const;

			// Utils
			std::string							trim(const std::string& str) const;
			std::string							toLower(const std::string& str) const;
			std::string							toUpper(const std::string& str) const;
			std::string							expand_path(const std::string& path, const std::string current_path = "", bool expand_symbolic = true, bool weakly = true) const;
			std::string							temp_path() const;
			long								parse_size(const std::string &value) const;
			bool								parse_bool(const std::string &value) const;
			uint8_t								parse_loglevel(const std::string &value) const;

		private:

			// Variables
			std::set<std::string>										validSections;
			std::map<std::string, std::set<std::string>>				validKeys;
			std::map<std::string, std::map<std::string, std::string>>	defaultValues;
			std::map<std::string, std::map<std::string, ConfigEntry>>	sections;
			std::string													currentSection;

			std::map<std::string, std::string>							environment;
			std::map<std::string, std::string>							environmentConfig;
			std::vector<ErrorInfo>										errors;
			uint16_t													error_maxLevel;
			uint16_t													order;															

			// Initialize
			void						initialize();
			void						default_values();

			// Section
			bool						is_section(const std::string& line) const;
			std::string					section_type(const std::string& section) const;
			std::string					section_extract(const std::string& line) const;
			int							section_parse(const std::string& line, int line_number, std::string& filename);

			// Keys
			std::string					remove_comments(const std::string& line) const;
			bool						key_valid(const std::string& section, const std::string& key) const;
			int							key_parse(const std::string& line, int line_number, std::string& filename);

			// Globbing
			bool						globbing_has_glob(const std::string& path);
			bool						globbing_match_glob(const std::string& pattern, const std::string& text);
			std::string					globbing_glob_to_regex(const std::string& glob);
			std::vector<std::string>	globbing_expand_glob(const std::string& pattern);
			std::vector<std::string>	globbing_expand(const std::vector<std::string>& patterns);

			// Include
			int							include_parse(const std::string& filePath);
			std::vector<std::string>	include_parse_files(const std::string& fileString, std::string& ConfigFile);
			void						include_process(std::string& ConfigFile);

			// Environment
			std::string					environment_apply_format(const std::string& value, const std::string& format) const;
			std::string					environment_apply_substring(const std::string& value, const std::string& substr_spec) const;
			std::string					environment_expand_expr(std::map<std::string, std::string>& env, const std::string& var_expr) const;
			std::string					environment_expand(std::map<std::string, std::string>& env, const std::string& str, std::string split = "") const;
			bool						environment_validate(const std::string& env_string) const;
			void						environment_del(std::map<std::string, std::string>& env, const std::string& key);
			void						environment_add(std::map<std::string, std::string>& env, const std::string& key, const std::string& value);
			void						environment_add(std::map<std::string, std::string>& env, const std::map<std::string, std::string>& src, bool overwrite = false);
			void						environment_add_batch(std::map<std::string, std::string>& env, const std::string& batch);
			std::string					environment_get(const std::map<std::string, std::string>& env, const std::string& key) const;
			void						environment_clone(std::map<std::string, std::string>& env, const std::map<std::string, std::string>& src);
			void						environment_initialize(std::map<std::string, std::string>& env);
			void						environment_print(const std::map<std::string, std::string>& env) const;

			// Validation
			bool						valid_bool(const std::string& value) const;
			bool						valid_number(const std::string& value, long min = 0, long max = 1024 * 1024 * 1024) const;
			bool						valid_path(const std::string& value, const std::string current_path = "", bool is_directory = false, bool allow_auto = false, bool allow_none = false) const;
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

			void						validate_taskmasterd();
			void						validate_program();
			void						validate_group();
			void						validate_unix_server();
			void						validate_inet_server();
			void						validate();

			// Error
			void						error_print();
			void						error_add(std::string& filename, std::string msg, uint8_t level, uint16_t line, uint16_t order);

			// Parser
			void						parse(const std::string& filePath = "");
			void						merge_options(const ConfigOptions& Options);

			// Utils
			std::string					config_path() const;
			int							check_fd_limit(uint16_t minfds) const;
			int							check_process_limit(uint16_t minprocs) const;
			bool						command_executable(const std::string& input, std::string& resolved) const;

	};

#pragma endregion
	
#pragma region "Variables"

	extern ConfigParser Config;

#pragma endregion
