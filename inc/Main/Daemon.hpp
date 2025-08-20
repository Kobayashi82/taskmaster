/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Daemon.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/14 23:10:46 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/14 23:12:46 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Methods"

	int daemonize();															// Detach the process from the terminal and run it in the background as a daemon

#pragma endregion
