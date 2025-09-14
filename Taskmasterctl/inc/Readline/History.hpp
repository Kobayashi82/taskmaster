/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   History.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/10 20:50:07 by vzurera-          #+#    #+#             */
/*   Updated: 2025/09/14 15:10:47 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include <string>
	#include <vector>

#pragma endregion

#pragma region "History"

	class History {

		private:

			// Variables
			size_t	position;
			bool	begining;
			bool	middle;

			std::vector<std::string> history;

		public:

			// Constructors
			History();
			History(const History&) = default;
			History(History&&) = default;
			~History() = default;

			// Overloads
			History& operator=(const History&) = default;
			History& operator=(History&&) = default;

			// Methods
			int			add(const std::string& line);
			void		clear();
			void		print(size_t offset, bool hide_events = true) const;

			// Get
			size_t		length() const;
			std::string	get(size_t pos) const;
			std::string	get_current() const;

			// Navigate
			std::string	prev();
			std::string	next();
			size_t		get_pos() const;
			void		set_pos(size_t pos);
			void		set_pos_end();

	};

#pragma endregion

extern History history;
