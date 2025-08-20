/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Authentication.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/18 13:40:01 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/18 13:46:05 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include <string>

#pragma endregion

#pragma region "Methods"

	int		get_userpass(const std::string msg, std::string & user, std::string & pass);	// Extracts username and password from a message
	bool	authenticate(const std::string& user, const std::string& pass);					// Authenticates a user with username and password

#pragma endregion
