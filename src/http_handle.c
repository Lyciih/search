#include "flow.h"

int http_handle(int connect_fd)
{
	char 	send_buffer[2000];
	char 	send_message[1000];
	char 	recv_packet_temp[1000];
	int 	read_state;
	FILE	* target;
	char	regex_result[100];
	
	read_state = read(connect_fd, recv_packet_temp, 999);
	if(read_state == 0)
	{
		perror("connect interrupt\n");
		return 0;
	}
	recv_packet_temp[read_state] = '\0';


	//用正則表達式取得用於 websocket 握手的key---------------------------------------------------------
	analysis(1, "Sec-WebSocket-Key: (.{24})", recv_packet_temp, regex_result, sizeof(regex_result));
	printf("%s\n", regex_result);


	printf("%s", recv_packet_temp);


	//開檔，讀取要回應的網頁---------------------------------------------------------------------------
	target = fopen("index.html", "r");

	fseek(target, 0, SEEK_END);
	long file_size = ftell(target);
	fseek(target, 0, SEEK_SET);

	fread(send_message, 1, file_size, target);
	send_message[file_size] = '\0';
	fclose(target);

	//產生http回應-------------------------------------------------------------------------------------
	sprintf(send_buffer,"HTTP/1.1 200 OK\r\n"
			"Context-Type: text/html\r\n"
			"Content-Length: %ld\r\n"
			"Connection: close\r\n"
			"\r\n"
			"%s\r\n"
			,file_size+2, send_message);
	
	//發送http回應，並結束連線-------------------------------------------------------------------------
	send(connect_fd, send_buffer, strlen(send_buffer), 0);
	close(connect_fd);
	return 0;
}
