# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/08/11 19:13:18 by vzurera-          #+#    #+#              #
#    Updated: 2025/09/02 22:17:11 by vzurera-         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

all:taskmasterd taskmasterctl

taskmasterd: daemon
server: daemon
daemon:
	@mkdir -p bin
	@$(MAKE) -s -C Taskmasterd all
	@-rm -f bin/taskmasterd
	@cp Taskmasterd/taskmasterd bin/

taskmasterctl: client
client:
	@mkdir -p bin
	@$(MAKE) -s -C Taskmasterctl all
	@-rm -f bin/taskmasterctl
	@cp Taskmasterctl/taskmasterctl bin/

re:
	@$(MAKE) -s -C Taskmasterd re
	@$(MAKE) -s -C Taskmasterctl re

clean:
	@$(MAKE) -s -C Taskmasterd clean
	@$(MAKE) -s -C Taskmasterctl clean

fclean:
	@$(MAKE) -s -C Taskmasterd fclean
	@$(MAKE) -s -C Taskmasterctl fclean
	@rm -rf bin

wipe:
	@$(MAKE) -s -C Taskmasterd wipe
	@$(MAKE) -s -C Taskmasterctl wipe
	@rm -rf bin

.PHONY: all taskmasterd daemon server taskmasterctl client fclean wipe re
