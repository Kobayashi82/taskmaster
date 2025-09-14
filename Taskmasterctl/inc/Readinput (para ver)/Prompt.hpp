/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Prompt.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 00:00:00 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/13 00:00:00 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>

namespace Terminal {

	enum class PromptType { 
		PS1, 
		PS2, 
		BOTH 
	};

	class Prompt {
	private:
		static std::string	prompt_ps1;
		static std::string	prompt_ps2;

	public:
		Prompt() = delete;  // Static class
		~Prompt() = delete;

		// Prompt management
		static void				set(PromptType type, const std::string& new_prompt);
		static void				clear(PromptType type);
		static const std::string&	get(PromptType type);
		
		// Initialization
		static int				initialize();
		
		// Cleanup
		static void				cleanup();
	};

}