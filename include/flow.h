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


int http_handle(int connect_fd);
int websocket_handle(int connect_fd);

int analysis(int capture_number, char * pattern, char * data, char * buffer, int buffer_size);

#endif

