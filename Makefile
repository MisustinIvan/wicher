CC = gcc
CFLAGS = -Wall -Wextra
NAME = wicher
SERVICE_FILE_NAME = $(NAME).service
SERVICE_FILE = /etc/systemd/system/$(SERVICE_FILE_NAME)

build: $(NAME)

$(NAME): *c
	$(CC) $(CFLAGS) -o $(NAME) *.c

clean:
	rm /usr/local/bin/$(NAME)

install: $(NAME) install_service
	cp $(NAME) /usr/local/bin

uninstall:
	rm -f /usr/local/bin/$(NAME)
	rm -f $(SERVICE_FILE)
	systemctl stop $(SERVICE_FILE_NAME) || true
	systemctl disable $(SERVICE_FILE_NAME) || true
	systemctl deamon-reload

install_service: $(SERVICE_FILE_NAME)
	cp ./$(SERVICE_FILE_NAME) $(SERVICE_FILE)
	systemctl daemon-reload
	systemctl enable $(SERVICE_FILE_NAME)
	systemctl start $(SERVICE_FILE_NAME)
