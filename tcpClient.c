/*socket tcp客户端*/
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
#include <unistd.h>

#define SERVER_PORT 5556

/*
 连接到服务器后，会不停循环，等待输入，
 输入quit后，断开与服务器的连接
 */
int ret = 0;
int recvSum, sendSum = 0;
int poll_timeout = 0;

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
	//客户端只需要一个套接字文件描述符，用于和服务器通信
	int clientSocket;
	//描述服务器的socket
	struct sockaddr_in serverAddr;
	char sendbuf[200];
	char recvbuf[200];
	int iDataNum;
	if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		return 1;
	}

	// 设置socket 为非阻塞
	fcntl(clientSocket, F_SETFL, O_NONBLOCK);

	struct pollfd fds;
	int ret = 0;
	int poll_timeout = 0;
	fds.fd = clientSocket;

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	//指定服务器端的ip，本地测试：127.0.0.1
	//inet_addr()函数，将点分十进制IP转换成网络字节序IP
	serverAddr.sin_addr.s_addr = inet_addr("192.168.92.2");
	ret = connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

	if (ret && (errno != EINPROGRESS))
	{
		if (ret < 0)
		{
			perror("connect");
			return 1;
		}
		perror("rconnect");
		if (clientSocket)
			close(clientSocket);
	}

	if (ret && (errno == EINPROGRESS))
	{
		perror("rconnect EINPROGRESS");
		fds.events = POLLOUT;
		ret = do_poll(&fds, poll_timeout);
		if (ret)
		{
			perror("rpoll");
			if (clientSocket)
				close(clientSocket);
		}
		socklen_t len;
		int err;
		len = sizeof err;
		ret = getsockopt(clientSocket, SOL_SOCKET, SO_ERROR, &err, &len);
		if (ret)
			if (clientSocket)
				close(clientSocket);
		if (err)
		{
			ret = -1;
			errno = err;
			perror("async rconnect");
			return ret;
		}
	}

	printf("connected with destination host...\n");
	// while (1)
	// {
	while (1)
	{
		printf("Input your world:>");
		scanf("%s", sendbuf);
		printf("\n");

		fds.events = POLLOUT;
		// printf(" will do_poll for send\n");
		// ret = do_poll(&fds, poll_timeout);
		// if (ret)
		// {
		// 	perror("do_poll err");
		// 	return ret;
		// }
		ret = poll(&fds, 1, poll_timeout);
		if (ret > 0 && (fds.revents & (POLLOUT | POLLHUP | POLLERR)))
		{
			if (fds.revents & POLLHUP)
			{
				printf("polled pollhup\n");
			}
			if (fds.revents & POLLERR)
			{
				printf("polled pollerr\n");
			}
			printf(" will rsend\n");
			// ret = rsend(client, buffer, iDataNum, 0);
			ret = send(clientSocket, sendbuf, strlen(sendbuf), 0);

			if (ret > 0)
			{
				sendSum += ret;
			}
			else if (errno != EWOULDBLOCK && errno != EAGAIN)
			{
				perror("rsend");
				return ret;
			}

			printf(" rsend return %d, data is %s\n", ret, sendbuf);
			break;
		}
		else
		{
			continue;
		}
	}

	printf("sleep 2 sec, then close socket %d\n", clientSocket);
	sleep(2);
	close(clientSocket);
	return 0;
}
