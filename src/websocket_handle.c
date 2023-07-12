#include "flow.h"


typedef struct websocket_frame{
	unsigned int mask2:8;
	unsigned int mask1:8;
	unsigned int payload_len:7;
	unsigned int mask:1;
	unsigned int opcode:4;
	unsigned int rsv3:1;
	unsigned int rsv2:1;
	unsigned int rsv1:1;
	unsigned int fin:1;
}websocket_frame;



int decimal_to_binary(int number, int digits, int end)
{
	int count = 0;
	for(int i = digits; i >= end; i--)
	{
		count++;
		printf("%d", (number >> i) & 1);
		if(count == 8)
		{
			printf(" ");
			count = 0;
		}
	}
	return 0;
}

int trans_ending(int size, char * source, char * result)
{
	for(int i = 0; i <= ((size + 1) + 4 - 1)/4; i++)
	{
		*((unsigned int *)result + i) = ntohl(*((unsigned int *)source + i));
	}
	return 0;
}

int print_frame_binary(int size, char * frame)
{
	int count = 0;
	for(int i = 0; i < (size + 1)/4; i++)
	{
		decimal_to_binary(((int *)frame)[i], 31, 0);
		count++;
		printf("\n");
	}

	if((size + 1) % 4 != 0)
	{
		decimal_to_binary(((int *)frame)[count], 31, 32 - (((size + 1) % 4) * 8));
	}



	return 0;
}

int websocket_handle(int connect_fd)
{
	char 	send_buffer[2000];
	char 	send_message[1000];
	char 	recv_packet_temp[1000];
	char 	local_ending[1000];
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

	while(1)
	{
		read_state = read(connect_fd, recv_packet_temp, 999);
		if(read_state == 0)
		{
			perror("connect interrupt\n");
			return 0;
		}


		recv_packet_temp[read_state] = '\0';
		trans_ending(read_state, recv_packet_temp, local_ending);
		websocket_frame * frame = (websocket_frame *)local_ending;

		/*
		printf("%d ", frame->fin);
		printf("%d ", frame->rsv1);
		printf("%d ", frame->rsv2);
		printf("%d ", frame->rsv3);
		decimal_to_binary(frame->opcode, 4);
		printf("% d ", frame->mask);
		decimal_to_binary(frame->payload_len, 7);
		printf(" ");
		decimal_to_binary(frame->mask_key_head, 16);
		printf("\n");
		decimal_to_binary(frame->mask_key_tail, 16);
		printf(" ");
		decimal_to_binary(frame->payload_data, 16);
		printf("\n");
		*/

		print_frame_binary(read_state, local_ending);
		printf("\n");
		printf("%d %d %d %d %d %d %d %d %d", 
				frame->mask1, 
				frame->mask2, 
				frame->payload_len,
				frame->mask,
				frame->opcode,
				frame->rsv3,
				frame->rsv2,
				frame->rsv1,
				frame->fin
				);
		printf("\n%d %d %d %d %d %d %d %d %d", 
				frame->fin,
				frame->rsv1,
				frame->rsv2,
				frame->rsv3,
				frame->opcode,
				frame->mask,
				frame->payload_len,
				frame->mask1,
				frame->mask2
				);
		printf("\n\n");
	}
	//close(connect_fd);
	return 0;
}
