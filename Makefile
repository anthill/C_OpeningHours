SHELL = /bin/bash

NAME = ./libopening-hours

P_INC = ./include/

SRCS = ./src/main.c			\
       ./src/is_open.c			\
       ./src/printing.c			\
       ./src/parsing.c			\
       ./src/wide_range_parsing.c	\
       ./src/small_range_parsing.c

MWAIT = echo -ne "\r\033[1;37m[  \033[31m....  \033[37m] \033[0m$$file"

MSKIP = echo -e "\r\033[1;37m[  \033[0;1;33mSKIP  \033[37m] \033[0m$$file"

MOK = echo -e "\r\033[1;37m[   \033[32mOK   \033[37m] \033[0m$$file"

MERR = echo -e "\r\033[1;37m[ \033[31mFAILED \033[37m] \033[0m$$file"

CFLAGS = -Iinclude/ -std=c99 -W -Wall -Wextra -g

LDFLAGS = -Llib/ -Iinclude/

CC = gcc

OBJS = $(SRCS:.c=.o)

CC_OBJS = $(addsuffix cc,$(OBJS))

RM = rm -f

MBIN = echo "Compiling binary..."

MOBJS = echo "Compiling objects..."

all:	lib

install:	lib
	cp $(NAME).so /usr/lib/
	cp ./include/opening_hours.h /usr/include
	cp ./include/bitset.h /usr/include

uninstall:
	rm /usr/lib/$(NAME).so
	rm /usr/include/opening_hours.h
	rm /usr/include/bitset.h

standalone: clean
	@$(MAKE) $(NAME) -j4 CFLAGS="$(CFLAGS) -DSTANDALONE" --no-print-directory | grep -v '^.ake.*$$'
	@echo
	@$(MAKE) clean 2>&1 >/dev/null

lib:
	@echo
	@set -o pipefail ; $(MAKE) --no-print-directory $(NAME).so -j4 NAME="$(NAME).so" CC="gcc -shared -fPIC" CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" 2>&1 | grep -v '^.ake.*$$' ; \
		if [ $${PIPESTATUS[0]} -gt "0" ] ; then \
			exit 1 ; \
		fi ;
	@echo -e "\n   Done\n"

$(NAME):	$(CC_OBJS)
	@file="$(NAME)" ; export relink="false" ; \
	for i in $(OBJS) ; do \
		if [[ "$$i" -nt "$(NAME)" ]] ; then export relink="true" ; fi ;\
	done ; \
	echo ; \
	if [ "$$relink" = "false" ] ; then $(MSKIP) ; exit 0 ; fi ; \
	$(MWAIT) ; $(CC) $(CFLAGS) $(LDFLAGS) -o $(NAME) $(OBJS) && $(MOK) || $(MERR)

test:
	@$(MAKE) $(NAME)-test -j4 NAME=$(NAME)-test CFLAGS="$(CFLAGS) -DDEBUG" LDFLAGS="$(LDFLAGS) -lcunit" SRCS="$(SRCS) ./src/tests.c" | grep -v '^.ake.*$$'
	@./$(NAME)-test

%.occ:
	@file="$*.c" ; set -o pipefail ; $(MAKE) CC="$(CC)" CFLAGS="$(CFLAGS)" --no-print-directory $*.o | grep "up to date" > /dev/null && $(MSKIP) && exit 0 ; \
		if [ $${PIPESTATUS[0]} -gt "0" ] ; then \
			$(MERR) ; \
			exit 1 ; \
		fi ; \
	$(MOK)

%.o:	%.c include/ Makefile
	@$(CC) $(CFLAGS) -o $@ -c $<

clean:
	@echo
	@file="Cleaning objects..." ; $(MWAIT) ; $(RM) $(OBJS) && $(MOK) || $(MERR)
	@echo

fclean:	clean
	@file="Cleaning $(NAME)" ; $(MWAIT) ; $(RM) $(NAME) $(NAME).so $(NAME)-test && $(MOK) || $(MERR)
	@echo

re:	fclean all

gyver:
	sl

.PHONY:	all re fclean test gyver
