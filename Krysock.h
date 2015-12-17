#ifndef KRYSOCKET_H
#define KRYSOCKET_H
#include "Krys.h"
#include <stdio.h>
#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h> 
#include <string.h>
#include <arpa/inet.h>
#include <strings.h>
#include <signal.h>
#include <netdb.h>
#include <string>
#include <sys/un.h>





#define READ_TIMEOUT 5
#define BUFFSIZE 1024

#ifdef __cplusplus
extern "C" {
#endif
int udp_bind (const char* ip, unsigned short port);

int tcp_listen (const char* host, unsigned short port);

char* socket_ip (int socket_);

unsigned short socket_port (int socket_);

int tcp_open (const char* host, unsigned short port);

int tcp_read (int socket, void* buf, int len);

int tcp_write (int socket, const void* buf, int len);

int recv_peek (int socket, void* buf, size_t len);

ssize_t tcp_readline (int socket, void* buf, int max_len);

int tcp_read_timeout (int socket_, void* buf, int len, int second);

int read_timeout (int socket, unsigned int sec);

int analysis_addr (const char *_addrstr, u_int32_t *_addr);

int krys_read (int socket_, void** buf);

int krys_write (int socket_, const void* buf, int len);


int world_saved ();

int send_fd (int sock, int fd_sent);

int recv_fd (const int sock_fd);

int fd_obtain (int fd, const char* socket_file);

int unlimit_fd (int max);

#ifdef __cplusplus
}
#endif
#ifdef __cplusplus
int java_read (int socket_, std::unique_ptr<char[]>& buf, int time_out);

int java_write (int socket_, const void* buf, unsigned short len);

std::string string_receive (int socket_, int timeout);

bool string_send (int socket_, const std::string& str);

#endif

#endif /*KRYSOCKET_H*/
