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

typedef struct node
{
	struct sockaddr_in clientaddr;
	struct node *next;
}linknode, *linklist;

void do_login(int sockfd, linklist H, Msg msg, struct sockaddr_in clientaddr);
void do_quit(int sockfd, linklist H, Msg msg, struct sockaddr_in clientaddr);
void do_chat(int sockfd, linklist H, Msg msg, struct sockaddr_in clientaddr);

void do_parent(int sockfd, struct sockaddr_in serveraddr);
void do_child(int sockfd);
linklist create_node(void);

int main(int argc, char *argv[])
{
	int sockfd;
	struct sockaddr_in serveraddr;

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
		perror("fail to socket ..");
		exit(1);
	}

	if (bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(struct sockaddr_in)) < 0)
	{
		perror("fail to bind ..");
		exit(1);
	}
	
	switch (fork())
	{
	case -1:
		perror("fail to fork");
		exit(1);

	case 0:
		do_child(sockfd);
		break;

	default:
		do_parent(sockfd, serveraddr);
		break;
	}

	return 0;
}

void do_parent(int sockfd, struct sockaddr_in serveraddr)
{
	Msg msg;
	strcpy(msg.name, "server");
	msg.type = 'B';

	while (1)
	{
		printf("system message >>");
		fgets(msg.text, N, stdin);
		msg.text[strlen(msg.text) - 1] = '\0';
		sendto(sockfd, &msg, sizeof(Msg), 0, (struct sockaddr*)&serveraddr, sizeof(struct sockaddr_in));
	}

	return;
}

void do_child(int sockfd)
{
	Msg msg;
	struct sockaddr_in clientaddr;
	socklen_t len = sizeof(struct sockaddr_in);
	memset(&clientaddr, 0, len);

	//创建链表头节点
	linklist H = NULL;
	H = create_node();

	if (H == NULL)
		exit(1);

	while (1)
	{
		recvfrom(sockfd, &msg, sizeof(Msg), 0, (struct sockaddr*)&clientaddr, &len);

		switch (msg.type)
		{
		case 'L':
			do_login(sockfd, H, msg, clientaddr);
			break;
		case 'B':
			do_chat(sockfd, H, msg, clientaddr);
			break;
		case 'Q':
			do_quit(sockfd, H, msg, clientaddr);
			break;
		}
	}

	return ;
}

linklist create_node(void)
{
	linklist head = NULL;

	head = (linklist)malloc(sizeof(linknode));
	if (head == NULL)
	{
		perror("fail to create node ..");
		return NULL;
	}

	head->next = NULL;
	return head;
}

void do_login(int sockfd, linklist H, Msg msg, struct sockaddr_in clientaddr)
{
	sprintf(msg.text, "%s login ...", msg.name);
	socklen_t len = sizeof(clientaddr);

	linklist p = NULL, q;
	q = H->next;

	//新用户的节点
	p = (linklist)malloc(sizeof(linknode));
	if (p == NULL)
	{
		perror("fail to create new node");
		return;
	}
	p->clientaddr = clientaddr;

	//给其他用户发送新用户的登录消息
	while (q)
	{
		sendto(sockfd, &msg, sizeof(Msg), 0, (struct sockaddr*)&(q->clientaddr), len);
		q = q->next;
	}

	//将新用户插入链表, 头插入
	q = H->next;
	p->next = q;
	H->next = p;

	return;
}

void do_chat(int sockfd, linklist H, Msg msg, struct sockaddr_in clientaddr)
{

	linklist p = H->next;
	socklen_t len = sizeof(clientaddr);
	char buf[N];
	/*
	 *sprintf左右参数不可以重复:比如
	 * sprintf(msg.text, "%s say %s", msg.name, msg.text);
	 *
	 */
	sprintf(buf , "%s say %s", msg.name, msg.text);
	strcpy(msg.text, buf);

	while (p)
	{
		if (memcmp(&(p->clientaddr), &clientaddr, sizeof(struct sockaddr_in)) != 0 )
		{
			sendto(sockfd, &msg, sizeof(Msg), 0, (struct sockaddr*)&(p->clientaddr), len);
		}

		p = p->next;
	}

	return;
}

void do_quit(int sockfd, linklist H, Msg msg, struct sockaddr_in clientaddr)
{
	sprintf(msg.text, "%s log out.. ", msg.name);
	linklist p = H, q;
	socklen_t len = sizeof(clientaddr);

	while (p->next)
	{
		if (memcmp(&(p->next->clientaddr), &clientaddr, sizeof(struct sockaddr_in)) != 0 )
		{
			sendto(sockfd, &msg, sizeof(Msg), 0, (struct sockaddr*)&(p->next->clientaddr), len);
			p = p->next;
		}
		else
		{
			//删除退出的用户节点
			q = p->next;
			p->next = q->next;
			free(q);
			q = NULL;
			//此处不需要进行 p = p->next, 否则会导致少判断一个节点
		}
	}

	return;
}




