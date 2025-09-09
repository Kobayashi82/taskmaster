/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Signals.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 18:09:55 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/09 18:16:31 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include <csignal>

#pragma endregion

class Signals {

	private:

		static volatile sig_atomic_t signum;

		static void	sigint_handler(int sig);
		static void	sigterm_handler(int sig);
		static void	sighup_handler(int sig);
		static void	sigquit_handler(int sig);
		static void	sigsev_handler(int sig);
		static void	sigpipe_handler(int sig);
		static void	sigchld_handler(int sig);

	public:

		int	set();

};