#include "flow.h"

int main(int argc, char ** argv)
{
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

	while(1)
	{
		if((connect_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &len)) < 0)
		{
			if(errno = EINTR)
			{
				continue;
			}
			else
			{	perror("accept error\n");
				exit(0);
			}
		}
		else	//連線成功
		{
			//顯示客戶端地址
			inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
			client_port = client_addr.sin_port;
			printf("accept success from %s %d\n", client_ip, client_port);

			websocket_handle(connect_fd);
		}
	}

	return 0;
}
