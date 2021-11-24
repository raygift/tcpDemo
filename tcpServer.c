/*socket tcp服务器端*/
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <rdma/rdma_cma.h>
#include <rdma/rsocket.h>

static const char *port = "5556";
static const char *src_addr = "192.168.92.2";
/*
 监听后，一直处于accept阻塞状态，
 直到有客户端连接，
 当客户端如数quit后，断开与客户端的连接
 */
static int rs, lrs;
int poll_timeout = 0;
int sendSum, recvSum = 0;
int ret = 0;

int do_poll(struct pollfd *fds, int timeout)
{
	int ret;

	do
	{
		ret = poll(fds, 1, timeout);
	} while (!ret);

	return ret == 1 ? (fds->revents & (POLLERR | POLLHUP)) : ret;
}

int main()
{
	//调用socket函数返回的文件描述符
	int serverSocket;
	//声明两个套接字sockaddr_in结构体变量，分别表示客户端和服务器
	struct sockaddr_in server_addr;
	struct sockaddr_in clientAddr;
	int addr_len = sizeof(clientAddr);
	int client;
	char buffer[200];
	int iDataNum;

	//socket函数，失败返回-1
	//int socket(int domain, int type, int protocol);
	//第一个参数表示使用的地址类型，一般都是ipv4，AF_INET
	//第二个参数表示套接字类型：tcp：面向连接的稳定数据传输SOCK_STREAM
	//第三个参数设置为0
	struct addrinfo *ai;
	// static struct rdma_addrinfo rai_hints;
	static struct addrinfo ai_hints;
	ai_hints.ai_socktype = SOCK_STREAM;
	ai_hints.ai_flags |= AI_PASSIVE;
	ret = getaddrinfo(src_addr, port, &ai_hints, &ai);
	if (ret)
	{
		printf("getaddrinfo: %s\n", gai_strerror(ret));
		return ret;
	}
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket < 0)
	{
		perror("socket");
		return 1;
	}
	// if (errno == ENOENT)
	// {
	// 	printf("serverSocket %d\n", serverSocket);
	// 	perror(" oh no such file or directory");
	// 	return 1;
	// }

	bzero(&server_addr, sizeof(server_addr));
	// //初始化服务器端的套接字，并用htons和htonl将端口和地址转成网络字节序
	// server_addr.sin_family = AF_INET;
	// server_addr.sin_port = htons(SERVER_PORT);
	// //ip可是是本服务器的ip，也可以用宏INADDR_ANY代替，代表0.0.0.0，表明所有地址
	// // server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	// server_addr.sin_addr.s_addr = inet_addr("192.168.92.2");
	// //对于bind，accept之类的函数，里面套接字参数都是需要强制转换成(struct sockaddr *)
	// //bind三个参数：服务器端的套接字的文件描述符，
	ret = bind(serverSocket, ai->ai_addr, ai->ai_addrlen);
	// rbind(serverSocket, (struct sockaddr *)&server_addr, sizeof(server_addr))

	if (ret)
	{
		perror("rbind err");
		return 1;
	}
	//设置服务器上的socket为监听状态
	if (listen(serverSocket, 5) < 0)
	{
		perror("listen");
		return 1;
	}
	printf("Listening on: %s %s\n", inet_ntoa(server_addr.sin_addr), port);
	// listen fd 设为非阻塞
	fcntl(serverSocket, F_SETFL, O_NONBLOCK);
	// while (1)
	// {

		struct pollfd lfds;
		lfds.fd = serverSocket;
		lfds.events = POLLIN;
		do
		{
			printf("will rpoll for accept(%d)\n", serverSocket);

			ret = do_poll(&lfds, poll_timeout);
			if (ret)
			{
				perror("rpoll");
				return ret;
			}

			client = accept(serverSocket, (struct sockaddr *)&clientAddr, (socklen_t *)&addr_len);
		} while (client < 0 && (errno == EAGAIN || errno == EWOULDBLOCK));
		if (client < 0)
		{
			perror("raccept");
			return client;
		}
		fcntl(client, F_SETFL, O_NONBLOCK);

		//inet_ntoa   ip地址转换函数，将网络字节序IP转换为点分十进制IP
		//表达式：char *inet_ntoa (struct in_addr);
		printf("IP is %s\n", inet_ntoa(clientAddr.sin_addr));
		printf("Port is %d\n", htons(clientAddr.sin_port));
		printf("\n");
		struct pollfd afds;
		int poll_timeout = 0;
		afds.fd = client;
		while (1)
		{
			while (1)
			{
				// sleep(1);
				// printf("recv client data...\n");

				// 循环poll 直到获取到pollin 事件
				afds.events = POLLIN;
				// printf("will do_poll for recv\n");
				// ret = do_poll(&afds, poll_timeout);
				// if (ret)
				// {
				// 	printf("do_poll ret: %d\n", ret);
				// 	perror("do_poll err");
				// 	return ret;
				// }
				ret = poll(&afds, 1, poll_timeout);
				if (ret > 0)
				{
					// if (afds.revents & ( POLLHUP | POLLERR))
					// {
					// }
					printf("polled %d\n", afds.revents);

					printf("will rrecv\n");
					iDataNum = recv(client, buffer, 1024, 0);
					if (iDataNum == 0)
					{
						printf("rrecvd 0, will close\n");
						// goto client_close;
					}
					if (iDataNum > 0)
					{
						printf("rrecvd %d\n", iDataNum);
						recvSum += iDataNum;
					}
					else if (errno != EWOULDBLOCK && errno != EAGAIN)
					{
						perror("rrecv error");
						return iDataNum;
					}

					buffer[iDataNum] = '\0';
					if (strcmp(buffer, "quit") == 0)
						return iDataNum;
					printf("rrecv bytes: %d, data is %s\n\n", iDataNum, buffer);
					break;
				}
				else
				{
					if (afds.revents & (POLLHUP | POLLERR))
					{
						printf("polled pollhup/pollerr\n");
					}
					// printf("poll for recv ret %d\n", ret);
					continue;
				}
			}
			sleep(5); //-- kill client
			while (1)
			{
				// printf("will send data to client...\n");
				afds.events = POLLOUT;
				// printf("will do_poll for send\n");
				// ret = do_poll(&afds, poll_timeout);
				// if (ret)
				// {
				// 	perror("do_poll error");
				// 	return ret;
				// }
				ret = poll(&afds, 1, poll_timeout);
				if (ret > 0)
				{
					printf("polled %d\n", afds.revents);
					printf("will rsend\n");
					ret = send(client, buffer, iDataNum, 0);

					if (ret > 0)
					{
						sendSum += ret;
					}
					else if (errno != EWOULDBLOCK && errno != EAGAIN)
					{
						perror("rsend error");
						return ret;
					}

					printf("rsend bytes: %d, data is %s\n\n", iDataNum, buffer);
					break;
				}
				else
				{
					// printf("poll for send ret %d\n", ret);

					continue;
				}
			}
		}

	client_close:
		close(client);
	// }
	close(serverSocket);

	return 0;
}
