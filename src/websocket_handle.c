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

	char tail;
	char head;
	unsigned int mask4:8;
	unsigned int mask3:8;
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

int receive_trans_ending(int size, char * source, char * result)
{
	for(int i = 0; i <= ((size + 1) + 4 - 1)/4; i++)
	{
		*((unsigned int *)result + i) = ntohl(*((unsigned int *)source + i));
	}
	return 0;
}

int send_trans_ending(int size, websocket_frame * source, char * result)
{
	for(int i = 0; i <= ((size + 1) + 4 - 1)/4; i++)
	{
		*((unsigned int *)result + i) = htonl(*((unsigned int *)source + i));
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

int payload_decode(char * head, int len, char * mask, char * buffer)
{
	char * current = head + 2;
	int state = 2;
	int count = 0;
	for(int i = 0; i < len; i++)
	{
		count++;
		//printf("%c", current[0 - state] ^ mask[i%4]);
		buffer[i] = current[0 - state] ^ mask[i%4];
		state++;
		if(state == 4)
		{
			current = current + 4;
			state = 0;
		}
	}
	buffer[count] = '\0';
	return 0;
}

int payload_send_sort(char * head, char * source, int len)
{
	char * current = head + 2;
	int state = 2;
	for(int i = 0; i <= len; i++)
	{
		current[0 - state] = source[i];
		state++;
		if(state == 4)
		{
			current = current + 4;
			state = 0;
		}
	}
	return 0;
}

int websocket_handshake(int connect_fd)
{
	char 	send_buffer[2000];
	char 	recv_packet_temp[1000];
	int 	read_state;
	//FILE	* target;
	char	regex_result[100];
	char	Sec_WebSocket_Accept_buffer[100];


	//讀取收到的請求---------------------------------------------------------
	read_state = read(connect_fd, recv_packet_temp, 999);
	if(read_state == -1)
	{
		perror("connect interrupt\n");
		return -1;
	}
	recv_packet_temp[read_state] = '\0';

	//檢查請求是否合法------------------------------------------------------
	if(analysis(1, "Upgrade: websocket", recv_packet_temp, regex_result, sizeof(regex_result)) != 0)
	{
		close(connect_fd);
		return -2;
	}

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
	/*
	target = fopen("index.html", "r");

	fseek(target, 0, SEEK_END);
	long file_size = ftell(target);
	fseek(target, 0, SEEK_SET);

	fread(send_message, 1, file_size, target);
	send_message[file_size] = '\0';
	fclose(target);
	*/
	//產生http回應-------------------------------------------------------------------------------------
	sprintf(send_buffer,
			"HTTP/1.1 101 Switching Protocols\r\n"
			"Upgrade: websocket\r\n"
			"Connection: Upgrade\r\n"
			"Sec-WebSocket-Accept: %s\r\n"
			"\r\n"
			, Sec_WebSocket_Accept_buffer);
	
	//發送http回應-------------------------------------------------------------------------
	send(connect_fd, send_buffer, strlen(send_buffer), 0);


	return 0;
}











int websocket_handle(int connect_fd, int epoll_fd)
{
	char 	send_buffer[2000];
	char 	recv_packet_temp[1000];
	char 	local_ending[1000];
	int 	read_state;
	char	frame_mask[4];
	char	decode_string[1000];
	websocket_frame send_frame;


	read_state = read(connect_fd, recv_packet_temp, 999);
	if(read_state == -1)
	{
		perror("read error\n");
		if(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, connect_fd, NULL) == -1)
		{
			perror("epoll_ctl remove socket fails\n");
			exit(1);
		}
		close(connect_fd);
		return -1;
	}

	if(read_state == 0)
	{

		if(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, connect_fd, NULL) == -1)
		{
			perror("epoll_ctl remove socket fails\n");
			exit(1);
		}
		close(connect_fd);
		return 0;
	}

	recv_packet_temp[read_state] = '\0';
	receive_trans_ending(read_state, recv_packet_temp, local_ending);
	websocket_frame * receive_frame = (websocket_frame *)local_ending;

	frame_mask[0] = receive_frame->mask1;
	frame_mask[1] = receive_frame->mask2;
	frame_mask[2] = receive_frame->mask3;
	frame_mask[3] = receive_frame->mask4;




	print_frame_binary(read_state, local_ending);
	printf("\n");
	printf("\n%d %d %d %d %d %d %d %d %d %d %d", 
			receive_frame->fin,
			receive_frame->rsv1,
			receive_frame->rsv2,
			receive_frame->rsv3,
			receive_frame->opcode,
			receive_frame->mask,
			receive_frame->payload_len,
			receive_frame->mask1,
			receive_frame->mask2,
			receive_frame->mask3,
			receive_frame->mask4
			);
	printf("\n\n");

	send_frame.fin = 1;
	send_frame.rsv1 = 0;
	send_frame.rsv2 = 0;
	send_frame.rsv3 = 0;
	send_frame.opcode = 1;
	send_frame.mask = 0;

	if(receive_frame->opcode == 8)
	{
		payload_decode(&(receive_frame->head), receive_frame->payload_len, frame_mask, decode_string);
		if(receive_frame->payload_len > 2)
		{

			printf("\n");

			char temp = decode_string[0];
			decode_string[0] = decode_string[1];
			decode_string[1] = temp;
			//decimal_to_binary(*((int *)(decode_string)), 31, 0);
			printf("%hd ", *((short int *)(decode_string)));
			printf("%s\n", decode_string + 2);
		}

		send_frame.opcode = 8;
		send_frame.payload_len = 0;
		send_trans_ending(2 , &send_frame, send_buffer);
		send(connect_fd, send_buffer, 2, 0);

		
		if(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, connect_fd, NULL) == -1)
		{
			perror("epoll_ctl remove socket fails\n");
			exit(1);
		}
		close(connect_fd);
	}
	else
	{

		payload_decode(&(receive_frame->head), receive_frame->payload_len, frame_mask, decode_string);
		printf("\n");
		printf("%s\n", decode_string);
		

		int send_len =  strlen(decode_string);
		payload_send_sort(&send_frame.head - 4, decode_string, send_len);
		send_frame.payload_len = send_len;
		print_frame_binary(2 + send_len, (char *)(&send_frame));
		printf("\n----------------------------\n");
		send_trans_ending(2 + send_len, &send_frame, send_buffer);
		send(connect_fd, send_buffer, 2 + send_len, 0);
	}
	
	return 0;
}
