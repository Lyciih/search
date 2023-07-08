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


int http_handle(int connect_fd);

#endif

