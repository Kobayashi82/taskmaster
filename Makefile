# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/08/11 19:13:18 by vzurera-          #+#    #+#              #
#    Updated: 2025/08/20 13:32:46 by vzurera-         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# ──────────── #
# ── COLORS ── #
# ──────────── #

NC    				= \033[0m
RED     			= \033[0;31m
GREEN   			= \033[0;32m
YELLOW  			= \033[0;33m
BLUE    			= \033[0;34m
MAGENTA 			= \033[0;35m
CYAN    			= \033[0;36m
WHITE   			= \033[0;37m
INV_RED  			= \033[7;31m
INV_GREEN	  		= \033[7;32m
INV_YELLOW  		= \033[7;33m
INV_BLUE  			= \033[7;34m
INV_MAGENTA			= \033[7;35m
INV_CYAN			= \033[7;36m
INV_WHITE			= \033[7;37m
BG_CYAN				= \033[40m
FG_YELLOW			= \033[89m
COUNTER 			= 0

# ────────── #
# ── NAME ── #
# ────────── #

NAME		= taskmaster
CTL_NAME	= taskmasterctl

# ─────────── #
# ── FLAGS ── #
# ─────────── #

CC			= clang++
CFLAGS		= -Wall -Wextra -Werror -std=c++17
CEFLAGS		= -lm -lcrypt

# ───────────────── #
# ── DIRECTORIES ── #
# ───────────────── #

INC_DIR		= inc/
BLD_DIR		= build/
OBJ_DIR		= $(BLD_DIR)obj/
SRC_DIR		= src/

# ─────────── #
# ── FILES ── #
# ─────────── #

SRCS		= Main/Main.cpp Main/Daemon.cpp Main/Shell.cpp		\
																\
			  Config/Options.cpp Config/Signals.cpp				\
			  Config/Logging.cpp								\
																\
			  Security/Authentication.cpp						\
			  Security/Encryption.cpp							\
																\
			  Network/Socket.cpp Network/Client.cpp				\
			  Network/Epoll.cpp Network/Communication.cpp

SRCS		:= $(addprefix Taskmaster/, $(SRCS))


CTL_SRCS	= Main/Main.cpp Main/Daemon.cpp Main/Shell.cpp		\
																\
			  Config/Options.cpp Config/Signals.cpp				\
			  Config/Logging.cpp								\
																\
			  Security/Authentication.cpp						\
			  Security/Encryption.cpp							\
																\
			  Network/Socket.cpp Network/Client.cpp				\
			  Network/Epoll.cpp Network/Communication.cpp

CTL_SRCS	:= $(addprefix TaskmasterCTL/, $(CTL_SRCS))

# ───────────────────────────────────────────────────────────── #
# ─────────────────────────── RULES ─────────────────────────── #
# ───────────────────────────────────────────────────────────── #

all: _show_title_all
$(NAME): _show_title_server
$(CTL_NAME): _show_title_ctl

SRC_PATHS	= $(addprefix $(SRC_DIR), $(SRCS))
OBJS		= $(SRCS:Taskmaster/%.cpp=$(OBJ_DIR)Taskmaster/%.o)
DEPS		= $(OBJS:.o=.d)


CTL_SRC_PATHS	= $(addprefix $(SRC_DIR), $(CTL_SRCS))
CTL_OBJS		= $(CTL_SRCS:TaskmasterCTL/%.cpp=$(OBJ_DIR)TaskmasterCTL/%.o)
CTL_DEPS		= $(CTL_OBJS:.o=.d)

-include $(DEPS)
-include $(CTL_DEPS)

_compile: $(OBJS)
	@$(MAKE) -s _hide_cursor
#	Create folder
	@mkdir -p $(BLD_DIR)

#	Compile
	@printf "\r%50s\r\t$(CYAN)Compiling... $(YELLOW)$(NAME)$(NC)"
	@$(CC) $(CFLAGS) -o $(NAME) $(OBJS) $(CEFLAGS)
	@printf "\r%50s\r\t$(CYAN)Compiled    $(GREEN)✓ $(YELLOW)$(NAME)$(NC)\n"

	@$(MAKE) -s _progress; printf "\n"
	@$(MAKE) -s _show_cursor

_compile_ctl: $(CTL_OBJS)
	@$(MAKE) -s _hide_cursor
#	Create folder
	@mkdir -p $(BLD_DIR)

#	Compile
	@printf "\r%50s\r\t$(CYAN)Compiling... $(YELLOW)$(CTL_NAME)$(NC)"
	@$(CC) $(CFLAGS) -o $(CTL_NAME) $(CTL_OBJS) $(CEFLAGS)
	@printf "\r%50s\r\t$(CYAN)Compiled    $(GREEN)✓ $(YELLOW)$(CTL_NAME)$(NC)\n"

	@$(MAKE) -s _progress; printf "\n"
	@$(MAKE) -s _show_cursor

_compile_all: $(OBJS)
	@printf "\n\t$(WHITE)──────────────────────────$(NC)\033[1A\r";
	@$(MAKE) -s $(CTL_OBJS)

	@$(MAKE) -s _hide_cursor
#	Create folder
	@mkdir -p $(BLD_DIR)

#	Compile Taskmaster
	@printf "\r%50s\r\t$(CYAN)Compiling... $(YELLOW)$(NAME)$(NC)"
	@$(CC) $(CFLAGS) -o $(NAME) $(OBJS) $(CEFLAGS)
	@printf "\r%50s\r\t$(CYAN)Compiled    $(GREEN)✓ $(YELLOW)$(NAME)$(NC)\n"

	@$(MAKE) -s _progress

#	Compile TaskmasterCTL
	@printf "\r%50s\r\t$(CYAN)Compiling... $(YELLOW)$(CTL_NAME)$(NC)"
	@$(CC) $(CFLAGS) -o $(CTL_NAME) $(CTL_OBJS) $(CEFLAGS)
	@printf "\r%50s\r\t$(CYAN)Compiled    $(GREEN)✓ $(YELLOW)$(CTL_NAME)$(NC)\n"

	@$(MAKE) -s _progress; printf "\n"
	@$(MAKE) -s _show_cursor

# ───────────── #
# ── OBJECTS ── #
# ───────────── #

-include $(DEPS)

$(OBJ_DIR)Taskmaster/%.o: $(SRC_DIR)Taskmaster/%.cpp
	@$(MAKE) -s _hide_cursor
#	Create folder
	@mkdir -p $(@D)

#	Compile objects
	@filename=$$(basename $<); filename=$${filename%.*}; \
	BAR=$$(printf "/ — \\ |" | cut -d" " -f$$(($(COUNTER) % 4 + 1))); \
	printf "\r%50s\r\t$(CYAN)Compiling... $(GREEN)$$BAR $(YELLOW)$$filename$(NC)"; \
	$(eval COUNTER=$(shell echo $$(($(COUNTER)+1))))
	@$(CC) $(CFLAGS) -I$(INC_DIR) -MMD -o $@ -c $<

$(OBJ_DIR)TaskmasterCTL/%.o: $(SRC_DIR)TaskmasterCTL/%.cpp
	@$(MAKE) -s _hide_cursor
#	Create folder
	@mkdir -p $(@D)

#	Compile objects
	@filename=$$(basename $<); filename=$${filename%.*}; \
	BAR=$$(printf "/ — \\ |" | cut -d" " -f$$(($(COUNTER) % 4 + 1))); \
	printf "\r%50s\r\t$(CYAN)Compiling... $(GREEN)$$BAR $(YELLOW)$$filename$(NC)"; \
	$(eval COUNTER=$(shell echo $$(($(COUNTER)+1))))
	@$(CC) $(CFLAGS) -I$(INC_DIR) -MMD -o $@ -c $<

# ───────────────── #
# ── EXTRA RULES ── #
# ───────────────── #

_show_title:
	@$(MAKE) -s _hide_cursor
	@$(MAKE) -s _title

#	Check if source exists and needs recompiling
	@if  [ ! -n "$(NAME)" ] || [ ! -n "$(SRCS)" ] || [ ! -d "$(SRC_DIR)" ]; then \
        printf "\t$(CYAN)Source files don't exist$(NC)"; \
		printf "\n\t$(WHITE)──────────────────────────$(NC)"; \
		$(MAKE) -s _progress; printf "\n" \
		$(MAKE) -s _show_cursor; \
	elif [ -f "$(NAME)" ] && \
		[ -z "$$(find $(SRC_PATHS) -newer "$(NAME)" 2>/dev/null; find inc -name '*.hpp' -newer "$(NAME)" 2>/dev/null)" ] && \
		[ $$(find $(OBJS) 2>/dev/null | wc -l) -eq $$(echo "$(OBJS)" | wc -w) ]; then \
        printf "\t$(GREEN)✓ $(YELLOW)$(NAME)$(CYAN) is up to date$(NC)"; \
		printf "\n\t$(WHITE)──────────────────────────$(NC)"; \
		$(MAKE) -s _progress; printf "\n" \
		$(MAKE) -s _show_cursor; \
	else \
		printf "\n\t$(WHITE)──────────────────────────$(NC)\033[1A\r"; \
		$(MAKE) -s _compile; \
	fi
	@$(MAKE) -s _show_cursor

_show_title_server:
	@$(MAKE) -s _hide_cursor
	@$(MAKE) -s _title_server

#	Check if source exists and needs recompiling
	@if  [ ! -n "$(NAME)" ] || [ ! -n "$(SRCS)" ] || [ ! -d "$(SRC_DIR)" ]; then \
        printf "\t$(CYAN)Source files don't exist$(NC)"; \
		printf "\n\t$(WHITE)──────────────────────────$(NC)"; \
		$(MAKE) -s _progress; printf "\n" \
		$(MAKE) -s _show_cursor; \
	elif [ -f "$(NAME)" ] && \
		[ -z "$$(find $(SRC_PATHS) -newer "$(NAME)" 2>/dev/null; find inc -name '*.hpp' -newer "$(NAME)" 2>/dev/null)" ] && \
		[ $$(find $(OBJS) 2>/dev/null | wc -l) -eq $$(echo "$(OBJS)" | wc -w) ]; then \
        printf "\t$(GREEN)✓ $(YELLOW)$(NAME)$(CYAN) is up to date$(NC)"; \
		printf "\n\t$(WHITE)──────────────────────────$(NC)"; \
		$(MAKE) -s _progress; printf "\n" \
		$(MAKE) -s _show_cursor; \
	else \
		printf "\n\t$(WHITE)──────────────────────────$(NC)\033[1A\r"; \
		$(MAKE) -s _compile; \
	fi
	@$(MAKE) -s _show_cursor

_show_title_ctl:
	@$(MAKE) -s _hide_cursor
	@$(MAKE) -s _title_ctl

#	Check if source exists and needs recompiling
	@if  [ ! -n "$(CTL_NAME)" ] || [ ! -n "$(CTL_SRCS)" ] || [ ! -d "$(SRC_DIR)" ]; then \
        printf "\t$(CYAN)Source files don't exist$(NC)"; \
		printf "\n\t$(WHITE)──────────────────────────$(NC)"; \
		$(MAKE) -s _progress; printf "\n" \
		$(MAKE) -s _show_cursor; \
	elif [ -f "$(CTL_NAME)" ] && \
		[ -z "$$(find $(CTL_SRC_PATHS) -newer "$(CTL_NAME)" 2>/dev/null; find inc -name '*.hpp' -newer "$(CTL_NAME)" 2>/dev/null)" ] && \
		[ $$(find $(CTL_OBJS) 2>/dev/null | wc -l) -eq $$(echo "$(CTL_OBJS)" | wc -w) ]; then \
        printf "\t$(GREEN)✓ $(YELLOW)$(CTL_NAME)$(CYAN) is up to date$(NC)"; \
		printf "\n\t$(WHITE)──────────────────────────$(NC)"; \
		$(MAKE) -s _progress; printf "\n" \
		$(MAKE) -s _show_cursor; \
	else \
		printf "\n\t$(WHITE)──────────────────────────$(NC)\033[1A\r"; \
		$(MAKE) -s _compile_ctl; \
	fi
	@$(MAKE) -s _show_cursor

_show_title_all:
	@$(MAKE) -s _hide_cursor
	@$(MAKE) -s _title_all

#	Check if sources exist
	@if  [ ! -n "$(NAME)" ] || [ ! -n "$(SRCS)" ] || [ ! -d "$(SRC_DIR)" ]; then \
        printf "\t$(CYAN)Source files don't exist$(NC)"; \
		printf "\n\t$(WHITE)──────────────────────────$(NC)"; \
		$(MAKE) -s _progress; printf "\n" \
		$(MAKE) -s _show_cursor; \
	elif [ -f "$(NAME)" ] && [ -f "$(CTL_NAME)" ] && \
		[ -z "$$(find $(SRC_PATHS) $(CTL_SRC_PATHS) -newer "$(NAME)" -o -newer "$(CTL_NAME)" 2>/dev/null; find inc -name '*.hpp' -newer "$(NAME)" -o -newer "$(CTL_NAME)" 2>/dev/null)" ] && \
		[ $$(find $(OBJS) $(CTL_OBJS) 2>/dev/null | wc -l) -eq $$(echo "$(OBJS) $(CTL_OBJS)" | wc -w) ]; then \
        printf "\t$(GREEN)✓ $(YELLOW)$(NAME) & $(CTL_NAME)$(CYAN) are up to date$(NC)"; \
		printf "\n\t$(WHITE)──────────────────────────$(NC)"; \
		$(MAKE) -s _progress; printf "\n" \
		$(MAKE) -s _show_cursor; \
	else \
		printf "\n\t$(WHITE)──────────────────────────$(NC)\033[1A\r"; \
		$(MAKE) -s _compile_all; \
	fi
	@$(MAKE) -s _show_cursor

# ───────────────────────────────────────────────────────────── #
# ───────────────────────── RE - CLEAN ─────────────────────────#
# ───────────────────────────────────────────────────────────── #

# ───────────── #
# ── RE-MAKE ── #
# ───────────── #

re:
	@$(MAKE) -s _hide_cursor
	@$(MAKE) -s _title_all

#	Check if source exists and needs recompiling
	@if  [ ! -n "$(NAME)" ] || [ ! -n "$(SRCS)" ] || [ ! -d "$(SRC_DIR)" ]; then \
        printf "\t$(CYAN)Source files don't exist$(NC)"; \
		printf "\n\t$(WHITE)──────────────────────────$(NC)"; \
		$(MAKE) -s _progress; \
		$(MAKE) -s _show_cursor; \
	fi

#	Delete objects
	@$(MAKE) -s _delete_objects
	@if [ -f $(NAME) ]; then \
		printf "\t$(CYAN)Deleting... $(YELLOW)$(NAME)$(NC)"; \
		rm -f $(NAME); \
	fi;
	@printf "\r%50s\r\t$(CYAN)Deleted     $(GREEN)✓ $(YELLOW)$(NAME)$(NC)\n";
	@$(MAKE) -s _progress;
	@if [ -f $(CTL_NAME) ]; then \
		printf "\t$(CYAN)Deleting... $(YELLOW)$(CTL_NAME)$(NC)"; \
		rm -f $(CTL_NAME); \
	fi;
	@printf "\r%50s\r\t$(CYAN)Deleted     $(GREEN)✓ $(YELLOW)$(CTL_NAME)$(NC)\n";
	@find $(BLD_DIR) -type d -empty -delete >/dev/null 2>&1 || true
	@$(MAKE) -s _progress; printf "\n"
	@printf "\t$(WHITE)──────────────────────────\n$(NC)"
	@printf "\033[1A\033[1A\r"

#	Compile both
	@$(MAKE) -s _compile_all

# ─────────── #
# ── CLEAN ── #
# ─────────── #

clean:
	@$(MAKE) -s _hide_cursor
	@$(MAKE) -s _title_all

	@$(MAKE) -s _delete_objects
	@printf "\r%50s\r\t$(CYAN)Deleted     $(GREEN)✓ $(YELLOW)objects$(NC)\n"

	@$(MAKE) -s _progress; printf "\n"
	@$(MAKE) -s _show_cursor

# ──────────── #
# ── FCLEAN ── #
# ──────────── #

fclean:
	@$(MAKE) -s _hide_cursor
	@$(MAKE) -s _title_all

	@$(MAKE) -s _delete_objects
	@if [ -f $(NAME) ]; then \
		printf "\t$(CYAN)Deleting... $(YELLOW)$(NAME)$(NC)"; \
		rm -f $(NAME); \
	fi;
	@printf "\r%50s\r\t$(CYAN)Deleted     $(GREEN)✓ $(YELLOW)$(NAME)$(NC)\n";
	@$(MAKE) -s _progress;
	@if [ -f $(CTL_NAME) ]; then \
		printf "\t$(CYAN)Deleting... $(YELLOW)$(CTL_NAME)$(NC)"; \
		rm -f $(CTL_NAME); \
	fi;
	@printf "\r%50s\r\t$(CYAN)Deleted     $(GREEN)✓ $(YELLOW)$(CTL_NAME)$(NC)\n"; \
	@find $(BLD_DIR) -type d -empty -delete >/dev/null 2>&1 || true
	@find  -type d -empty -delete >/dev/null 2>&1 || true

	@$(MAKE) -s _progress; printf "\n"
	@$(MAKE) -s _show_cursor

# ───────────────────────────────────────────────────────────── #
# ───────────────────────── FUNCTIONS ───────────────────────── #
# ───────────────────────────────────────────────────────────── #

# ─────────── #
# ── TITLE ── #
# ─────────── #

_title:
	@clear
	@printf "\n$(NC)\t$(INV_CYAN) $(BG_CYAN)$(FG_YELLOW)★$(INV_CYAN) $(BG_CYAN)$(FG_YELLOW)★$(INV_CYAN) $(BG_CYAN)$(FG_YELLOW)★\
	$(INV_CYAN)  $(NC)$(INV_CYAN)$(shell echo $(NAME) | tr a-z A-Z | tr '_' ' ')$(INV_CYAN)  \
	$(BG_CYAN)$(FG_YELLOW)★$(INV_CYAN) $(BG_CYAN)$(FG_YELLOW)★$(INV_CYAN) $(BG_CYAN)$(FG_YELLOW)★$(INV_CYAN) $(NC)\n"
	@printf "\t$(WHITE)──────────────────────────\n$(NC)"

_title_server:
	@clear
	@printf "\n$(NC)\t$(INV_CYAN) $(BG_CYAN)$(FG_YELLOW)★$(INV_CYAN) $(BG_CYAN)$(FG_YELLOW)★$(INV_CYAN) $(BG_CYAN)$(FG_YELLOW)★\
	$(INV_CYAN)  $(NC)$(INV_CYAN)$(shell echo $(NAME) | tr a-z A-Z | tr '_' ' ')$(INV_CYAN)  \
	$(BG_CYAN)$(FG_YELLOW)★$(INV_CYAN) $(BG_CYAN)$(FG_YELLOW)★$(INV_CYAN) $(BG_CYAN)$(FG_YELLOW)★$(INV_CYAN) $(NC)\n"
	@printf "\t$(WHITE)──────────────────────────\n$(NC)"

_title_ctl:
	@clear
	@printf "\n$(NC)\t$(INV_MAGENTA) $(BG_CYAN)$(FG_YELLOW)★$(INV_MAGENTA) $(BG_CYAN)$(FG_YELLOW)★$(INV_MAGENTA) $(BG_CYAN)$(FG_YELLOW)★\
	$(INV_MAGENTA) $(NC)$(INV_MAGENTA)$(shell echo $(CTL_NAME) | tr a-z A-Z | tr '_' ' ')$(INV_MAGENTA) \
	$(BG_CYAN)$(FG_YELLOW)★$(INV_MAGENTA) $(BG_CYAN)$(FG_YELLOW)★$(INV_MAGENTA) $(BG_CYAN)$(FG_YELLOW)★$(INV_MAGENTA) $(NC)\n"
	@printf "\t$(WHITE)──────────────────────────\n$(NC)"

_title_all:
	@clear
	@printf "\n$(NC)\t$(INV_GREEN) $(BG_CYAN)$(FG_YELLOW)★$(INV_GREEN) $(BG_CYAN)$(FG_YELLOW)★$(INV_GREEN) $(BG_CYAN)$(FG_YELLOW)★\
	$(INV_GREEN)  $(NC)$(INV_GREEN)$(shell echo $(NAME) | tr a-z A-Z | tr '_' ' ')$(INV_GREEN)  \
	$(BG_CYAN)$(FG_YELLOW)★$(INV_GREEN) $(BG_CYAN)$(FG_YELLOW)★$(INV_GREEN) $(BG_CYAN)$(FG_YELLOW)★$(INV_GREEN) $(NC)\n"
	@printf "\t$(WHITE)──────────────────────────\n$(NC)"

# ───────────── #
# ── CURSORS ── #
# ───────────── #

_hide_cursor:
	@printf "\e[?25l"

_show_cursor:
	@printf "\e[?25h"

# ──────────────────── #-
# ── DELETE OBJECTS ── #
# ──────────────────── #

_delete_objects:
	@printf "\n\t$(WHITE)──────────────────────────$(NC)\033[1A\r"
	@if [ -n "$(shell find $(OBJ_DIR) -type f -name '*.o' 2>/dev/null)" ]; then \
		COUNTER=0; \
		find $(OBJ_DIR) -type f -name '*.o' | while read -r file; do \
			BAR=$$(printf "/ — \\ |" | cut -d" " -f$$((COUNTER % 4 + 1))); \
			filename=$$(basename $$file); \
			srcpath=$$(find $(SRC_DIR) -type f -name "$${filename%.o}.*" 2>/dev/null); \
			if [ -n "$$srcpath" ]; then \
				rm -f $$file $$(dirname $$file)/$${filename%.o}.d; \
				filename=$${filename%.*}; \
				printf "\r%50s\r\t$(CYAN)Deleting... $(GREEN)$$BAR $(YELLOW)$$filename$(NC)"; sleep 0.05; \
				COUNTER=$$((COUNTER+1)); \
			fi; \
		done; \
	fi; printf "\r%50s\r"
	@-find $(BLD_DIR) -type d -empty -delete >/dev/null 2>&1 || true

wipe:
	@rm -rf $(BLD_DIR)
	@rm -rf $(NAME)
	@rm -rf $(CTL_NAME)

# ─────────────────── #
# ── PROGRESS LINE ── #
# ─────────────────── #

_progress:
	@total=26; printf "\r\t"; for i in $$(seq 1 $$total); do printf "$(RED)─"; sleep 0.01; done; printf "$(NC)"
	@total=26; printf "\r\t"; for i in $$(seq 1 $$total); do printf "─"; sleep 0.01; done; printf "\n$(NC)"

# ─────────── #
# ── PHONY ── #
# ─────────── #

.PHONY: all clean fclean re wipe $(CTL_NAME) _show_title _show_title_server _show_title_ctl _show_title_all _title _title_server _title_ctl _title_all _hide_cursor _show_cursor _delete_objects _progress _progress_line _compile _compile_ctl _compile_all
