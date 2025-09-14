/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Prompt.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 00:00:00 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/14 12:54:37 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Readline/Prompt.hpp"
#include "Readline/Termcaps.hpp"
#include <algorithm>
#include <regex>

// Color definitions - these would normally come from a colors header
#define BLUE600  "\033[38;5;27m"
#define GREEN600 "\033[38;5;34m"
#define RED600   "\033[38;5;196m"
#define Y        "\033[33m"
#define NC       "\033[0m"

// TODO: This should be properly integrated later
struct OptionsInfo {
    bool multiwidth_chars = false;
};

static OptionsInfo options __attribute__((unused));

namespace Terminal {

    // Static member definitions
    std::string Prompt::prompt_ps1;
    std::string Prompt::prompt_ps2;

    // Default prompts - using color definitions from the C code
    constexpr const char* PS1_DEFAULT = BLUE600 "[" GREEN600 "kobayashi" BLUE600 "]" GREEN600 "-" RED600 "42" Y "sh" BLUE600 "> " NC;
    constexpr const char* PS2_DEFAULT = "> ";

    // Helper function to process backslashes
    std::string processBackslashes(const std::string& str) {
        std::string result = str;
        size_t pos = 0;
        
        while ((pos = result.find("\\\\", pos)) != std::string::npos) {
            result.erase(pos, 1);  // Remove one backslash
            pos += 1;
        }
        
        return result;
    }

    // Helper function to remove forbidden characters
    void removeForbiddenChars(std::string& new_prompt) {
        if (new_prompt.empty()) {
            return;
        }

        std::string result;
        result.reserve(new_prompt.length());
        
        for (size_t i = 0; i < new_prompt.length(); ) {
            size_t char_size = Termcaps::getCharSize(static_cast<unsigned char>(new_prompt[i]));
            
            if (i + char_size > new_prompt.length()) {
                break;
            }
            
            if (char_size == 0) {
                i++;
                continue;
            }

            if (Termcaps::getCharWidth(i, new_prompt) < 2) {
                result.append(new_prompt.substr(i, char_size));
            }
            
            i += char_size;
        }
        
        new_prompt = result;
    }

    void Prompt::set(PromptType type, const std::string& new_prompt) {
        clear(type);

        std::string tmp_prompt;
        
        if (!new_prompt.empty()) {
            tmp_prompt = new_prompt;
        } else {
            switch (type) {
                case PromptType::PS1:
                    tmp_prompt = processBackslashes(PS1_DEFAULT);
                    break;
                case PromptType::PS2:
                    tmp_prompt = processBackslashes(PS2_DEFAULT);
                    break;
                default:
                    return;
            }
        }

        // Remove forbidden characters if multi-width characters are not supported
        if (!options.multiwidth_chars) {
            removeForbiddenChars(tmp_prompt);
        }

        switch (type) {
            case PromptType::PS1:
                prompt_ps1 = tmp_prompt;
                break;
            case PromptType::PS2:
                prompt_ps2 = tmp_prompt;
                break;
            case PromptType::BOTH:
                // This shouldn't happen in set, but handle it anyway
                break;
        }
    }

    void Prompt::clear(PromptType type) {
        switch (type) {
            case PromptType::PS1:
                prompt_ps1.clear();
                break;
            case PromptType::PS2:
                prompt_ps2.clear();
                break;
            case PromptType::BOTH:
                prompt_ps1.clear();
                prompt_ps2.clear();
                break;
        }
    }

    const std::string& Prompt::get(PromptType type) {
        switch (type) {
            case PromptType::PS1:
                return prompt_ps1;
            case PromptType::PS2:
                return prompt_ps2;
            default:
                return prompt_ps1;  // Default fallback
        }
    }

    int Prompt::initialize() {
        set(PromptType::PS1, "");  // Empty string will use default
        set(PromptType::PS2, "");  // Empty string will use default
        return 0;
    }

    void Prompt::cleanup() {
        clear(PromptType::BOTH);
    }

}