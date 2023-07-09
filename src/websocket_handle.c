#include "flow.h"

int websocket_handle(int connect_fd)
{
	char 	send_buffer[2000];
	char 	send_message[1000];
	char 	recv_packet_temp[1000];
	int 	read_state;
	FILE	* target;
	char	regex_result[100];
	char	Sec_WebSocket_Accept_buffer[100];


	read_state = read(connect_fd, recv_packet_temp, 999);
	if(read_state == 0)
	{
		perror("connect interrupt\n");
		return 0;
	}
	recv_packet_temp[read_state] = '\0';


	//用正則表達式取得用於 websocket 握手的key---------------------------------------------------------
	if(analysis(1, "Sec-WebSocket-Key: (.{24})", recv_packet_temp, regex_result, sizeof(regex_result)) == 0)
	{


		//strcpy(regex_result, "dGhlIHNhbXBsZSBub25jZQ==");    //for test,and final answer is s3pPLMBiTxaQ9kYGzzhZRbK+xOo=
		strcat(regex_result, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
		SHA_CTX sha1;
		SHA1_Init(&sha1);
		SHA1_Update(&sha1, regex_result, strlen(regex_result));
		unsigned char hash[SHA_DIGEST_LENGTH];
		SHA1_Final(hash, &sha1);


		BIO * bmem = BIO_new(BIO_s_mem());
		BIO * b64  = BIO_new(BIO_f_base64());
		BIO * bio  = BIO_push(b64, bmem);

		BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

		BIO_write(bio, hash, SHA_DIGEST_LENGTH);
		BIO_flush(bio);

		BUF_MEM * bptr;
		BIO_get_mem_ptr(bio, &bptr);
		strcpy(Sec_WebSocket_Accept_buffer, bptr->data);


		BIO_free_all(bio);
		
		printf("%s\n", regex_result);
		printf("%s\n", hash);
		printf("%s\n", Sec_WebSocket_Accept_buffer);
	}




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
	sprintf(send_buffer,
			"HTTP/1.1 101 Switching Protocols\r\n"
			"Upgrade: websocket\r\n"
			"Connection: Upgrade\r\n"
			"Sec-WebSocket-Accept: %s\r\n"
			"\r\n"
			, Sec_WebSocket_Accept_buffer);
	
	//發送http回應，並結束連線-------------------------------------------------------------------------
	send(connect_fd, send_buffer, strlen(send_buffer), 0);
	//close(connect_fd);
	return 0;
}
