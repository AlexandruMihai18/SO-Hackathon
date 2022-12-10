#ifndef _IPC_H
#define _IPC_H

#define BUFSIZE 1024
#define MAX_CLIENTS 1024
#define SOCKET_NAME "/tmp/sohack.socket"

int create_socket();
int create_server();
int connect_socket(int);
int accept_socket(int fd, int max_clients);
ssize_t send_socket(int, const char *, size_t);
ssize_t recv_socket(int, char *buf, size_t);
void close_socket(int);

#endif /* _IPC_H */
