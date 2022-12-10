#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <netinet/in.h>

#include "ipc.h"

static struct sockaddr_in addr;
static struct sockaddr cli_addr;
static unsigned short port = 4242;

int create_socket()
{
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;
	int ret = bind(fd, (struct sockaddr*)&addr, sizeof(addr));
	if (ret < 0) {
		return -1;
	}
	return fd;
}

int connect_socket(int fd)
{
	listen(fd, 5);
	int addrlen = sizeof(struct sockaddr);
	return accept(fd, &cli_addr, &addrlen);
	
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

