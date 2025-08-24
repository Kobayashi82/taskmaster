/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/24 17:00:33 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/24 19:55:23 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

	#include <string>															// std::string
	#include <cstdint>															// uint16_t

	std::string	expand_path(const std::string& path);
	std::string	temp_path();
	std::string	config_path();

	int	check_fd_limit(uint16_t minfds);
	int	check_process_limit(uint16_t minprocs);

	// std::string	trim(const std::string& str);
	// std::string	toLower(const std::string& str);
