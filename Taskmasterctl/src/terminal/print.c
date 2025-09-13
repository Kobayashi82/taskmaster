/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/01 12:18:06 by vzurera-          #+#    #+#             */
/*   Updated: 2025/01/27 12:04:19 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "libft.h"
	#include "print.h"

	#define INITIAL_CAP 1024		//	Initial capacity of each buffer

#pragma endregion

#pragma region "Variables"

	static char		*msg[1024];		//	Pointer to the buffer storing content for each file descriptor
	static size_t	len[1024];		//	Current size of the content stored for each file descriptor
	static size_t	cap[1024];		//	Total allocated capacity of the buffer for each file descriptor

#pragma endregion

#pragma region "Print"

	//	Manages and writes content to a dynamic buffer associated with a file descriptor
	//
	//	Parameters:
	//		- fd: The file descriptor to write to
	//		- str: The string to append to the buffer (can be NULL)
	//		- mode: Controls the behavior based on the following enum values:
	//			- RESET:			Resets the buffer
	//			- RESET_PRINT:		Resets the buffer and writes its content
	//			- FREE_RESET:		Frees the input string and resets the buffer
	//			- FREE_RESET_PRINT:	Frees the input string, resets the buffer, and writes its content
	//			- FREE_JOIN:		Frees the input string and appends its content to the buffer
	//			- FREE_PRINT:		Frees the input string and writes the buffer content
	//			- JOIN:				Appends the input string to the buffer
	//			- PRINT:			Writes the buffer content
	//			- RESET_ALL:		Resets all buffers
	//
	//	Returns: 0 on success, 1 on failure
	//
	int print(int fd, char *str, int mode) {
		if (fd < 0 || fd > 1023) return (1);

		if (mode == RESET_ALL) {
			//	Reset and free all buffers
			for (int i = 0; i < 1024; ++i) {
				sfree(msg[i]);
				len[i] = 0;
				cap[i] = 0;
				return (0);
			}
		}

		size_t str_len = str ? ft_strlen(str) : 0;

		//	Reset the buffer if necessary
		if ((mode >= RESET && mode <= FREE_RESET_PRINT) && msg[fd]) {
			sfree(msg[fd]);
			msg[fd] = NULL;
			len[fd] = 0;
			cap[fd] = 0;
		}

		//	If there is something to add
		if (str) {
			//	Initialize the buffer if it doesn't exist
			if (!msg[fd]) {
				cap[fd] = (str_len > INITIAL_CAP) ? str_len : INITIAL_CAP;
				msg[fd] = smalloc(cap[fd]);
				len[fd] = 0;
			}

			//	Resize the buffer if there's not enough space
			if (len[fd] + str_len >= cap[fd]) {
				size_t new_cap = cap[fd] * 2;
				while (len[fd] + str_len >= new_cap)
					new_cap *= 2;

				char *new_msg = ft_realloc(msg[fd], cap[fd], new_cap);
				msg[fd] = new_msg;
				cap[fd] = new_cap;
			}

			//	Copy the new string into the buffer
			ft_memcpy(msg[fd] + len[fd], str, str_len);
			len[fd] += str_len;
			msg[fd][len[fd]] = '\0';
		}

		//	Write the buffer if the mode indicates it
		int result = 0;
		if (mode % 2 && msg[fd] && fd > 0) {
			result = write(fd, msg[fd], len[fd]);
			sfree(msg[fd]);
			msg[fd] = NULL;
			len[fd] = 0;
			cap[fd] = 0;
		}

		//	Free the input string if necessary
		if ((mode >= FREE_RESET && mode <= FREE_PRINT) && str) sfree(str);

		return (result == -1);
	}

#pragma endregion
