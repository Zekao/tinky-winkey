# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: emaugale <emaugale@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/03/29 22:37:34 by emaugale          #+#    #+#              #
#    Updated: 2023/03/30 00:36:09 by emaugale         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

CC = cl.exe
CFLAGS = /Wall /WX

SRCS_WINKEY = winkey.c
SRCS_TINKY = tinky.c

OBJS_WINKEY = $(SRCS_WINKEY:.c=.obj)
OBJS_TINKY = $(SRCS_TINKY:.c=.obj)

all: winkey.exe tinky.exe

winkey.exe: $(OBJS_WINKEY)
    $(CC) $(CFLAGS) /Fe$@ $**
tinky.exe: $(OBJS_TINKY)
    $(CC) /Fe$@ $**

clean:
    del *.obj

fclean: clean
    del *.exe

re: fclean all

.c.obj:
    $(CC) $(CFLAGS) /c $<

.PHONY: all clean fclean re