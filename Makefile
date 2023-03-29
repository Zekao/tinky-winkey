# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: emaugale <emaugale@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/03/29 22:37:34 by emaugale          #+#    #+#              #
#    Updated: 2023/03/29 22:39:43 by emaugale         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

CC = cl.exe
CFLAGS = /Wall /WX

SRCS = winkey.c
OBJS = $(SRCS:.c=.obj)

all: winkey.exe

winkey.exe: $(OBJS)
    $(CC) $(CFLAGS) /Fe$@ $**

clean:
    del *.obj

fclean: clean
    del *.exe

re: fclean all

.c.obj:
    $(CC) $(CFLAGS) /c $<

.PHONY: all clean fclean re