/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Shell.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/12 18:02:22 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/18 22:26:57 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Methods"

	class Client;
	int	shell_start(Client *client);											// Start a new shell session
	int	shell_close(Client *client);											// Close a shell session

#pragma endregion
