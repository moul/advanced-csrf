NAME		=	acsrf

SRCDIR		=	./src
SRC		=	$(SRCDIR)/main.c    	\
			$(SRCDIR)/client.c	\
			$(SRCDIR)/net.c		\
			$(SRCDIR)/parser.c	\
			$(SRCDIR)/session.c	\
			$(SRCDIR)/timer.c	\
			$(SRCDIR)/http.c	\
			$(SRCDIR)/blind.c	\
			$(SRCDIR)/misc.c


CONFDIR		=	./confs

OBJ		=	$(SRC:.c=.o)

CC		=	gcc

INCLUDE		=	-I.
LIB		=
DEFINES		=	$(DEBUG)
#DEFINES		+=	-DNVERBOSE

CFLAGS		=	$(INCLUDE) \
			-W -Wall -Wstrict-prototypes \
			-ansi -pedantic \
			$(DEFINES)

LDFLAGS		=	$(LIB)

$(NAME)		:	$(OBJ)
			$(CC) -o $@ $(OBJ) $(LDFLAGS)
			chmod +x $@

all		:	$(NAME)

.c.o		:
			@echo $(CC) -o $@ -c $< $(CFLAGS)
			@printf "\033[0;31m"
			@$(CC) -o $@ -c $< $(CFLAGS)
			@printf "\033[0;0m"

clean		:
			rm -f $(OBJ) *~ include/*~ \#*

fclean		:	clean
			rm -f $(NAME) *.core
#			rm -f $(CONFDIR)/out.*

re		:	fclean all

.PHONY		:	clean fclean re
