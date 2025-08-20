/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Authentication.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/18 13:39:47 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/18 22:16:17 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "Config/Logging.hpp"
	#include "Security/Authentication.hpp"

	#include <cstring>															// std::strcmp()
	#include <unistd.h>															// crypt()
	#include <shadow.h>															// getspnam()

#pragma endregion

#pragma region "Get User/Pass"

	int get_userpass(const std::string msg, std::string & user, std::string & pass) {
		size_t userPos = msg.find(" user=");
		size_t passPos = msg.find(" pass=");

		if (userPos != std::string::npos && passPos != std::string::npos) {
			size_t userStart = userPos + 6;
			size_t userEnd   = passPos;
			user = msg.substr(userStart, userEnd - userStart);
			size_t passStart = passPos + 6;
			pass = msg.substr(passStart);

			return (0);
		}

		return (1);
	}

#pragma endregion

#pragma region "Authenticate"

	bool authenticate(const std::string& user, const std::string& pass) {
		auto spwd_entry = getspnam(user.c_str());
		if (!spwd_entry) return (false);

		const char* hash = spwd_entry->sp_pwdp;
		char* res = crypt(pass.c_str(), hash);
		if (!res) return (false);

		return (std::strcmp(res, hash) == 0);
	}

#pragma endregion
