#ifndef _TCPCLI_H_
#define _TCPCLI_H_

#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>

int tcp_connect(const char *host, const char *port, int timeout);
void tcp_close(int sock);
ssize_t tcp_send(int sock, const char *str);
ssize_t tcp_receive(int sock, char *buffer, unsigned int size);

#endif
