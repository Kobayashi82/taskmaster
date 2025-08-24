/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/24 16:23:28 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/24 19:24:58 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Options.hpp"
	#include "Config/Config.hpp"
	#include "Utils/Utils.hpp"

	#include <unistd.h>															// getuid()

#pragma endregion

#pragma region "Variables"

	Config::s_taskmasterd	Config::taskmasterd;

	bool					Config::is_root			= (getuid() == 0);			// 

#pragma endregion

Config::s_taskmasterd::s_taskmasterd() : 
    nodaemon(false),
    silent(false),
    user("do not switch"),
    umask(022),
    directory("do not change"),
    logfile(expand_path("supervisord.log")),
    logfile_maxbytes(50 * 1024 * 1024),
    logfile_backups(10),
    loglevel(INFO),
    pidfile(expand_path("supervisord.pid")),
    identifier("taskmaster"),
    childlogdir(temp_path()),
    strip_ansi(false),
    nocleanup(false),
    minfds(1024),
    minprocs(200)
{
	extern char **environ;
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


int Config::load_config(const std::string config_path) {
	if (config_path.empty()) return (1);

	return (0);
}

void Config::add_opt_args() {
	if (Options::options.find_first_of('n') != std::string::npos) taskmasterd.nodaemon			= Options::nodaemon;
	if (Options::options.find_first_of('s') != std::string::npos) taskmasterd.silent			= Options::silent;
	if (Options::options.find_first_of('u') != std::string::npos) taskmasterd.user				= Options::user;
	if (Options::options.find_first_of('m') != std::string::npos) taskmasterd.umask				= Options::umask;
	if (Options::options.find_first_of('d') != std::string::npos) taskmasterd.directory			= Options::directory;
	if (Options::options.find_first_of('l') != std::string::npos) taskmasterd.logfile			= Options::logfile;
	if (Options::options.find_first_of('y') != std::string::npos) taskmasterd.logfile_maxbytes	= Options::logfile_maxbytes;
	if (Options::options.find_first_of('z') != std::string::npos) taskmasterd.logfile_backups	= Options::logfile_backups;
	if (Options::options.find_first_of('e') != std::string::npos) taskmasterd.loglevel			= Options::loglevel;
	if (Options::options.find_first_of('j') != std::string::npos) taskmasterd.pidfile			= Options::pidfile;
	if (Options::options.find_first_of('i') != std::string::npos) taskmasterd.identifier		= Options::identifier;
	if (Options::options.find_first_of('q') != std::string::npos) taskmasterd.childlogdir		= Options::childlogdir;
	if (Options::options.find_first_of('t') != std::string::npos) taskmasterd.strip_ansi		= Options::strip_ansi;
	if (Options::options.find_first_of('k') != std::string::npos) taskmasterd.nocleanup			= Options::nocleanup;
	if (Options::options.find_first_of('a') != std::string::npos) taskmasterd.minfds			= Options::minfds;
	if (Options::options.find_first_of('p') != std::string::npos) taskmasterd.minprocs			= Options::minprocs;
}
