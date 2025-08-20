/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Encryption.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/15 23:36:28 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/20 13:58:46 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

	#include "Security/Encryption.hpp"

	#include <iomanip>															// std::stringstream, std::setfill(), std::setw()
	#include <shadow.h>															// getspnam()

	const std::string KEY	= "Th1s_1s_n0t_4_s3cr3t_k3y";						// Encryption key

#pragma region "Encryptation"

	#pragma region "Internal"

		#pragma region "Process"

			static std::string process(const std::string& data) {
				std::string result;
				result.reserve(data.length());

				for (size_t i = 0; i < data.length(); ++i) {
					char encrypted_char = data[i] ^ KEY[i % KEY.length()];
					result.push_back(encrypted_char);
				}

				return (result);
			}

		#pragma endregion

		#pragma region "To Hex"

			static std::string toHex(const std::string& data) {
				std::stringstream ss;
				ss << std::hex << std::setfill('0');
				for (unsigned char c : data) ss << std::setw(2) << static_cast<int>(c);

				return (ss.str());
			}

		#pragma endregion

		#pragma region "From Hex"

			static std::string fromHex(const std::string& hexData) {
				if (hexData.length() % 2 != 0) throw std::invalid_argument("Invalid hex string length");

				std::string result;
				result.reserve(hexData.length() / 2);

				for (size_t i = 0; i < hexData.length(); i += 2) {
					std::string byte = hexData.substr(i, 2);
					char c = static_cast<unsigned char>(std::strtol(byte.c_str(), nullptr, 16));
					result.push_back(c);
				}

				return (result);
			}

		#pragma endregion

	#pragma endregion

	#pragma region "Encrypt"

		std::string encrypt(const std::string& plaintext)  {
			return (toHex(process(plaintext)));
		}

	#pragma endregion

	#pragma region "Decrypt"

		std::string decrypt(const std::string& ciphertext) {
			return (process(fromHex(ciphertext)));
		}

	#pragma endregion

	#pragma region "Encrypt with continuous index"

		std::string encrypt_with_index(const std::string& plaintext, size_t& index) {
			std::string result;
			result.reserve(plaintext.length());

			for (size_t i = 0; i < plaintext.length(); ++i) {
				char encrypted_char = plaintext[i] ^ KEY[index % KEY.length()];
				result.push_back(encrypted_char);
				index++;
			}

			return (toHex(result));
		}

	#pragma endregion

	#pragma region "Decrypt with index"

		std::string decrypt_with_index(const std::string& ciphertext, size_t& index) {
			std::string binary_data = fromHex(ciphertext);
			std::string result;
			result.reserve(binary_data.length());

			for (size_t i = 0; i < binary_data.length(); ++i) {
				char decrypted_char = binary_data[i] ^ KEY[index % KEY.length()];
				result.push_back(decrypted_char);
				index++;
			}

			return (result);
		}

	#pragma endregion

#pragma endregion
