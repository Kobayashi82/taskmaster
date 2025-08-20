/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Encryption.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/16 11:54:52 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/18 13:46:12 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include <string>

#pragma endregion

#pragma region "Methods"

	std::string	encrypt(const std::string& plaintext);								// Encrypts a plaintext string
	std::string	decrypt(const std::string& ciphertext);								// Decrypts a ciphertext string
	std::string	encrypt_with_index(const std::string& plaintext, size_t& index);	// Encrypts with continuous index
	std::string	decrypt_with_index(const std::string& ciphertext, size_t& index);	// Decrypts with continuous index

#pragma endregion
