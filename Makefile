CC = gcc
CFLAGS = -Wall -Wextra
NAME = wicher

build: $(NAME)

$(NAME): *c
	$(CC) $(CFLAGS) -o $(NAME) *.c

clean:
	rm /usr/local/bin/$(NAME)

install:
	cp $(NAME) /usr/local/bin
