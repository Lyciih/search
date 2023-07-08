#include "flow.h"


char 	send_buffer[2000];
char 	send_message[1000];
char 	recv_packet_temp[1000];
char 	analysis[100];
int 	read_state;
FILE	* target;

int http_handle(int connect_fd)
{
	read_state = read(connect_fd, recv_packet_temp, 999);
	if(read_state == 0)
	{
		perror("connect interrupt\n");
		return 0;
	}
	recv_packet_temp[read_state] = '\0';

	//用正則表達式取得想要的標頭資料---------------------------------------------------------
	int reg_state;
	regex_t find;
	regmatch_t match[2];
	char pattern[] = "Sec-WebSocket-Key: (.{24})";
	reg_state = regcomp(&find, pattern, REG_EXTENDED);
	if(reg_state != 0)
	{
		printf("regex compile error\n");
		return 0;
	}

	reg_state = regexec(&find, recv_packet_temp, 2, match, 0);
	if(reg_state == REG_NOMATCH)
	{
		printf("no match\n");
	}
	else if(reg_state == 0)
	{
		for(int i = match[1].rm_so; i < match[1].rm_eo; i++)
		{
			printf("%c", recv_packet_temp[i]);
		}
		printf("\n");
	}

	regfree(&find);
	//---------------------------------------------------------------------------------------


	printf("%s", recv_packet_temp);

	target = fopen("index.html", "r");

	fseek(target, 0, SEEK_END);
	long file_size = ftell(target);
	fseek(target, 0, SEEK_SET);

	fread(send_message, 1, file_size, target);
	send_message[file_size] = '\0';
	fclose(target);

	sprintf(send_buffer,"HTTP/1.1 200 OK\r\n"
			"Context-Type: text/html\r\n"
			"Content-Length: %ld\r\n"
			"Connection: close\r\n"
			"\r\n"
			"%s\r\n"
			,file_size+2, send_message);
	send(connect_fd, send_buffer, strlen(send_buffer), 0);
	close(connect_fd);
	return 0;
}
