/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   options.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/13 22:27:45 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/11 19:26:03 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "main/options.h"

#pragma endregion

#pragma region "Strtoul"

	static int ft_strtoul(char **argv, const char *optarg, size_t *value, size_t max_value, bool allow_zero) {
		char *endptr;
		*value = strtoul(optarg, &endptr, 0);

		if (*endptr)							{ fprintf(stderr, "%s: invalid value (`%s' near `%s')\n", argv[0], optarg, endptr);	return (1); }
		if (!*value && !allow_zero)				{ fprintf(stderr, "%s: option value too small: %s\n", argv[0], optarg);				return (1); }
		if (max_value && *value > max_value)	{ fprintf(stderr, "%s: option value too big: %s\n", argv[0], optarg);				return (1); }

		return (0);
	}

#pragma endregion

#pragma region "Help"

	static int help() {
		fprintf(stderr, "Usage:\n");
		fprintf(stderr, "  %s ...\n", NAME);
		fprintf(stderr, "Options:\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "Report bugs to <kobayashi82@outlook.com>.\n");

		return (1);
	}

#pragma endregion

#pragma region "Usage"

	static int usage() {
		fprintf(stderr, "Usage: %s ...\n", NAME);

		return (1);
	}

#pragma endregion

#pragma region "Version"

	static int version() {
		fprintf(stderr, "%s 1.0 (based on ls (GNU coreutils) 9.4).\n", NAME);
		fprintf(stderr, "Copyright (C) 2025 Kobayashi Corp ⓒ.\n");
		fprintf(stderr, "License WTFPL: DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE.\n");
		fprintf(stderr, "This is free software: you are free to change and redistribute it.\n");
		fprintf(stderr, "There is NO WARRANTY, to the extent permitted by law.\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "Written by Kobayashi82 (vzurera-).\n");

		return (1);
	}

#pragma endregion

#pragma region "Invalid"

	static int invalid() {
		fprintf(stderr, "Try '%s --help' or '%s --usage' for more information.\n", NAME, NAME);
		return (2);
	}

#pragma endregion

#pragma region "Parse"

	int parse_options(t_options *options, int argc, char **argv) {
		memset(options, 0, sizeof(t_options));

		struct option long_options[] = {
			{"example",			required_argument,	0, 'e'},	// [-e, --example=VALUE]
			{"help",			no_argument,		0, 'h'},	// [-h?, --help]
			{"usage",			no_argument,		0, 'u'},	// [	--usage]
			{"version",			no_argument,		0, 'V'},	// [-V, --version]
			{0, 0, 0, 0}
		};

		options->pid = getpid();

		int opt;
		while ((opt = getopt_long(argc, argv, "ch?uV", long_options, NULL)) != -1) {
			switch (opt) {
				case 'e':	if (ft_strtoul(argv, optarg, &options->example, 0, true))						return (2);					break;
				case '?':	if (!strcmp(argv[optind - 1], "-?"))															return (help());	return (invalid());
				case 'h':																									return (help());
				case 'u':																									return (usage());
				case 'V':																									return (version());
			}
		}

		if (optind >= argc) {
			fprintf(stderr, "%s: missing argument\n", NAME);
			invalid(); return (2);
		}

		return (0);
	}

#pragma endregion
