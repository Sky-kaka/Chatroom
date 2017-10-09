#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#define  M  32
#define  N  128

typedef struct
{
	char type;
	char name[M];
	char text[N];
}Msg;

void do_child(int sockfd, struct sockaddr_in serveraddr, Msg msg);
void do_parent(int sockfd);

int main(int argc, const char *argv[])
{
	int sockfd;
	struct sockaddr_in serveraddr;
	Msg msg;

	if (argc < 3)
	{
		puts("input valid arg");
		exit(1);
	}

	memset(&serveraddr, 0, sizeof(struct sockaddr_in));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(atoi(argv[2]));
	serveraddr.sin_addr.s_addr = inet_addr(argv[1]);

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
	{
		perror("fail to socket");
		exit(1);
	}

	//登录
	printf("please input your name >>");
	fgets(msg.name, M, stdin);
	msg.name[strlen(msg.name) - 1] = '\0';

	msg.type = 'L';
	sendto(sockfd, &msg, sizeof(Msg), 0, (struct sockaddr*)&serveraddr, sizeof(struct sockaddr_in));

	switch (fork())
	{
	case -1:
		perror("fail to fork ..");
		exit(1);

	case 0:
		do_child(sockfd, serveraddr, msg);
		break;

	default:
		do_parent(sockfd);
		break;
	}


	return 0;
}

void do_child(int sockfd, struct sockaddr_in serveraddr, Msg msg)
{
	socklen_t len = sizeof(serveraddr);

	while (1)
	{
		printf(">>");
		fgets(msg.text, N, stdin);
		msg.text[strlen(msg.text) - 1] = '\0';

		if (strncmp(msg.text, "quit", 4) == 0)
		{
			msg.type = 'Q';
			sendto(sockfd, &msg, sizeof(Msg), 0, (struct sockaddr*)&serveraddr, len);
			kill(getppid(), SIGKILL);
			exit(0);
		}
		else
		{
			msg.type = 'B';
			sendto(sockfd, &msg, sizeof(Msg), 0, (struct sockaddr*)&serveraddr, len);
		}
	}

}

void do_parent(int sockfd)
{
	Msg msg;

	while (1)
	{
		recvfrom(sockfd, &msg, sizeof(Msg), 0, NULL, NULL);
		printf("%s\n", msg.text);
	}

}
