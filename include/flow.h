#ifndef FLOW_H_INCLUDE
#define FLOW_H_INCLUDE

#include <stdio.h>	//printf()
#include <stdlib.h>	//exit()
#include <string.h>	//memset()
#include <sys/socket.h>	//socket()
//#include <netinet/in.h>
#include <arpa/inet.h>	//sockaddr_in inet_pton()
#include <errno.h>	//errno
#include <unistd.h>	//read()
#include <regex.h>	//解析http

#include <openssl/sha.h>//sha-1
#include <openssl/bio.h>//base64
#include <openssl/evp.h>//base64
#include <openssl/buffer.h>//base64


#include <openssl/ssl.h>//SSL_library_init()
#include <openssl/bio.h>//SSL_load_error_strings()
#include <openssl/err.h>//OpenSSL_add_all_algorithms()



#include <sys/epoll.h>


#include "hiredis.h"

typedef struct	client{
	int	fd;
	SSL	*ssl;
}client;


int http_handle(int connect_fd);
int websocket_handshake(int connect_fd);
int websocket_handshake_ssl(int connect_fd, SSL * ssl);
int websocket_handle(int connect_fd, int epoll_fd);
int websocket_handle_ssl(client * current_data, int epoll_fd, client * client_info, int client_begin, int max_client);

int analysis(int capture_number, char * pattern, char * data, char * buffer, int buffer_size);

#endif

