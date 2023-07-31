#include "flow.h"

int main(int argc, char ** argv)
{

	redisContext * redis;
	redisReply * reply;

	redis = redisConnect("127.0.0.1", 6379);
	if(redis == NULL)
	{
		perror("redis connect error\n");
		redisFree(redis);
		exit(1);
	}
		//freeReplyObject(reply);
		redisFree(redis);




	if(argc != 3)
	{
		printf("arg need ip and port\n");
		exit(1);
	}
	
	//申請用來 listen 的 socket
	int	listen_fd = 0;	
	if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("create listen socket error\n");
		exit(1);
	}
	else
	{
		printf("create listen socket success\n");
	}

	//將 socket 設為地址可重用，這樣 server 中止後可立即重啟，不用等待 kernel 回收完
	int reuse_addr_on = 1;
	if(setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr_on, sizeof(reuse_addr_on)) < 0)
	{
		perror("listen address reuse failed\n");
		exit(1);
	}

	//建立用來填 server 地址的結構並填入
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	if(inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0)
	{
		printf("ip address error\n");
		exit(1);
	}
	server_addr.sin_port = htons(atoi(argv[2]));


	//把server的地址 bind 到 listen socket 上
	if((bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0))
	{
		perror("bind error\n");
		exit(1);
	}
	else
	{
		printf("bind success\n");
	}

	//開始listen
	if(listen(listen_fd, 5) < 0)
	{
		perror("listen error\n");
		exit(1);
	}
	else
	{
		printf("listen success\n");
	}

	//準備用來 accept 的結構，以及儲存客戶端地址的變數
	socklen_t	len;
	struct		sockaddr_in client_addr;
	char 		client_ip[INET_ADDRSTRLEN];
	unsigned short 	client_port;
	len 		= sizeof(client_addr);

	//用來存放連線socket描述符的變數
	int	connect_fd = 0;


	//建立 epoll
	int	epoll_fd = epoll_create(1001);
	if(epoll_fd == -1)
	{
		perror("epoll create fails\n");
		exit(1);
	}

	
	//將 listen socket 註冊到 epoll 中 
	struct epoll_event listen;
	listen.events = EPOLLIN;
	listen.data.fd = listen_fd;
	
	if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &listen) == -1)
	{
		perror("epoll_ctl add listen socket fails\n");
		exit(1);
	}


	//準備用來接收待處理事件的陣列
	int max_events = 1001;
	//struct epoll_event * get_events = malloc(max_events * sizeof(struct epoll_event));
	struct epoll_event get_events[max_events];
	int get_event_count;
	int current_fd;
	uint32_t current_event;
	

	//用來註冊新socket的epoll_event
	struct epoll_event test;


	// epoll_wait 迴圈
	while(1)
	{
		get_event_count = epoll_wait(epoll_fd, get_events, max_events, -1);
		if(get_event_count == -1)
		{
			perror("epoll_wait error\n");
			exit(1);
		}
		//printf("get %d event\n", get_event_count);

		for(int i = 0; i < get_event_count; i++)
		{
			current_fd = get_events[i].data.fd;
			current_event = get_events[i].events;

			if(current_fd == listen_fd)
			{
				connect_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &len);
				if(connect_fd == -1)
				{
					perror("accept error\n");
				}
				else	//連線成功
				{

					if(websocket_handshake(connect_fd) == 0)
					{
					
						//將新連線加入epoll中
						test.events = EPOLLIN;
						test.data.fd = connect_fd;
						
						if((epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connect_fd, &test)) == -1)
						{
							perror("epoll_ctl add client socket fails\n");
							exit(1);
						}
						else
						{
							//顯示客戶端地址
							inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
							client_port = client_addr.sin_port;
							printf("accept success from %s %d %d\n", client_ip, client_port, connect_fd);
						}
					}
				}
			}
			else if(current_event & EPOLLIN)
			{
				websocket_handle(current_fd, epoll_fd);
			}
		}
	}
	return 0;
}
