#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>

#include "ipc.h"

#define SOCKET_PATH "/tmp/mysocket"

unsigned int addrlen = sizeof(struct sockaddr_un);
struct sockaddr_un serv, client;

int create_socket()
{
	return socket(AF_UNIX, SOCK_STREAM, 0);
}

int create_server() {
	int fd = create_socket();
	memset(&serv, 0, sizeof(serv));
	serv.sun_family = AF_UNIX;
	strcpy(serv.sun_path, SOCKET_PATH);
	unlink(serv.sun_path);
	int len = strlen(serv.sun_path) + sizeof(serv.sun_family);
	int ret = bind(fd, (struct sockaddr *)&serv, (socklen_t)len);
	if (ret < 0) {
		return -1;
	}
	return fd;
}

int connect_socket(int fd)
{
	serv.sun_family = AF_UNIX;
	strcpy(serv.sun_path, SOCKET_PATH);
	int len = strlen(serv.sun_path) + sizeof(serv.sun_family);
	return connect(fd, (struct sockaddr *)&serv, (socklen_t)len);
}

int accept_socket(int fd) {
	listen(fd, MAX_CLIENTS);
	return accept(fd, (struct sockaddr *)&client, (socklen_t *)&addrlen);
}

ssize_t send_socket(int fd, const char *buf, size_t len)
{
	return send(fd, buf, len, 0);
}

ssize_t recv_socket(int fd, char *buf, size_t len)
{
	return recv(fd, buf, len, 0);
}

void close_socket(int fd)
{
	shutdown(fd, 2);
}

