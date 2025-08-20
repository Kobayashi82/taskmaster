# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/08/11 19:13:18 by vzurera-          #+#    #+#              #
#    Updated: 2025/08/20 14:09:44 by vzurera-         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

all:
taskmaster taskmasterctl

taskmaster: daemon
server: daemon
daemon:
	@mkdir -p bin
	@$(MAKE) -s -C Taskmaster all
	@cp Taskmaster/taskmaster bin/

taskmasterctl: client
client:
	@mkdir -p bin
	@$(MAKE) -s -C TaskmasterCTL all
	@cp TaskmasterCTL/taskmasterctl bin/

re:
	@$(MAKE) -s -C Taskmaster re
	@$(MAKE) -s -C TaskmasterCTL re

clean:
	@$(MAKE) -s -C Taskmaster clean
	@$(MAKE) -s -C TaskmasterCTL clean

fclean:
	@$(MAKE) -s -C Taskmaster fclean
	@$(MAKE) -s -C TaskmasterCTL fclean
	@rm -rf bin

wipe:
	@$(MAKE) -s -C Taskmaster wipe
	@$(MAKE) -s -C TaskmasterCTL wipe
	@rm -rf bin

.PHONY: all taskmaster daemon server taskmasterctl client fclean wipe re
