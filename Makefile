SHELL = /bin/bash

NAME = ./C_OpeningHours

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

CFLAGS = -Iinclude/ -std=c99 -DSTANDALONE -g -D_DEBUG

LDFLAGS = -Llib/ -Iinclude/

CC = gcc

OBJS = $(SRCS:.c=.o)

CC_OBJS = $(addsuffix cc,$(OBJS))

RM = rm -f

MBIN = echo "Compiling binary..."

MOBJS = echo "Compiling objects..."

all:
	@${MAKE} $(NAME) -j4 | sed 's/^make\[[0-9]\].*$$//'

$(NAME):	$(CC_OBJS)
	@file="$(NAME)" ; export relink="false" ; \
	for i in $(OBJS) ; do \
		if [[ "$$i" -nt "$(NAME)" ]] ; then export relink="true" ; fi ;\
	done ; \
	if [ "$$relink" = "false" ] ; then $(MSKIP) ; exit 0 ; fi ; \
	$(MWAIT) ; $(CC) $(CFLAGS) $(CFLAGS) -o $(NAME) $(OBJS) && $(MOK) || $(MERR)

test:
	$(CC) $(CFLAGS) -lcunit -o ./src/tests.o -c ./src/tests.c 
	$(CC) $(CFLAGS) $(LDFLAGS) -lcunit -o tests-unit ./src/tests.o
	./tests-unit

%.occ:
	@file="$*.c" ; set -o pipefail ; ${MAKE} --no-print-directory $*.o | grep "up to date" > /dev/null && $(MSKIP) && exit 0 ; \
		if [ $${PIPESTATUS[0]} -gt "0" ] ; then \
			$(MERR) ; \
			exit 1 ; \
		fi ; \
	$(MOK)

%.o:	%.c include/ Makefile
	@$(CC) $(CFLAGS) -o $@ -c $<

clean:
	@file="Cleaning objects..." ; $(MWAIT) ; $(RM) $(OBJS) && $(MOK) || $(MERR)

fclean:	clean
	@file="Cleaning $(NAME)" ; $(MWAIT) ; $(RM) $(NAME) && $(MOK) || $(MERR)

re:	fclean all

gyver:
	sl

.PHONY:	all re fclean test gyver
