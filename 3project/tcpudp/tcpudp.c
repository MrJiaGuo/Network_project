#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <pthread.h>
#include "../dlist/dlist.h"
#include "tcpudp.h"
#include "../bmp/aboutbmp.h"

/*

	服务器端，作为中间人，转发来自客户端的各种信息，文字消息，文件，图片！
	通过服务器端存储所有连接上网络的客户端！
	进行客户端上线的广播消息的发送！

*/


//定义tcp和udp套接字
int udpsock;
int tcpsocket;

extern tcp_t head;

//线程函数，没连上一个客户端都创建一个线程
void  *rdwr_client(void *arg)
{
	tcp_t newclient = (tcp_t)arg;
	
	char buf[100];
	int count;

	//定义*c按：分割字符串
	char *c = ":";
	
	//定义数组，分别存放收到的消息的类型，ip，sock
	char type[20];
	char ip1[50];
	char sock1[50];
	
	//定义数组存放消息内容
	char content[100];
	char str[100];
	char msg[50];
	
	//计算":"的数目
	int i;
	int num = 0;
	
while(1)
{
	while(1)
	{
		//定义结构体成员
		tcp_t p = head->next;
		tcp_t q = head->next;
		tcp_t k = head->next;
	
		//初始化数组
		bzero(type,sizeof(type));
		bzero(buf,sizeof(buf));
		bzero(ip1,sizeof(ip1));
		bzero(sock1,sizeof(sock1));
		bzero(content,sizeof(content));
		bzero(str,sizeof(str));
		bzero(msg,sizeof(msg));

		//等待客服端发来消息类型
		count = read(newclient->sock,buf,sizeof(buf));
				
		if(count==0)
		{
			printf("好友%d已经下线！\n",newclient->sock);
			delete_node(head,newclient->sock);
			pthread_exit(NULL);
		}	
		printf("buf:%s\n",buf);
		//广播消息类型
		if(strcmp(buf,"broadcast")==0)
		{
				printf("broadcast wait\n");
				count = recv(newclient->sock,msg,sizeof(msg),/*MSG_WAITALL*/0);
				if(count==0)
				{
					printf("好友%d已经下线！\n",newclient->sock);
					delete_node(head,newclient->sock);
					pthread_exit(NULL);
				}	
				//将发送方的ip和sock存储在str数组
				sprintf(str, "%s:%d: ",newclient->ip,newclient->sock );
				//拼接消息内容
				strcat(str,msg);
				
				printf("str:%s\n",str);
				
				bzero(type,sizeof(type));
				strcat(type,"broadcast");
				//循环向在线好友发送广播消息
				while(p != head)
				{
					if(p->sock != newclient->sock)
					{
						printf("type:%s\n",type);
						write(p->sock,type,strlen(type));
						usleep(50000);
						write(p->sock,str,strlen(str));
						printf("已经向好友%d发送\n",p->sock);
					}
					p = p->next;
				}

		}
			
		else
		{
			//把消息类型拆分为类型，ip，sock
			printf("strok1\n");
			strcpy(type,strtok(buf,c));
			strcpy(ip1,strtok(NULL,c));
			strcpy(sock1,strtok(NULL,c));
			int tmp = atoi(sock1);
			printf("strok2\n");

			
			//转发单播消息类型
			if(strcmp(type,"info")==0)
			{
				count = recv(newclient->sock,msg,sizeof(msg),/*MSG_WAITALL*/0);
				if(count==0)
				{
					printf("好友%d已经下线！\n",newclient->sock);
					delete_node(head,newclient->sock);
					pthread_exit(NULL);
				}	
				
				sprintf(str, "%s:%d: ",newclient->ip,newclient->sock );
		
				strcat(str,msg);
				
				printf("str:%s\n",str);
				
				//遍历链表转发信息
				while(p != head)
				{
					if(p->sock == tmp&& (strcmp(p->ip,ip1)==0))
					{
						strcat(type,":info");
						printf("type:%s\n",type);
						write(p->sock,type,strlen(type));
						usleep(50000);
						write(p->sock,str,strlen(str));
						printf("已经向好友%d发送\n",p->sock);
					}
					p = p->next;
				}

					
			}
			//转发文件消息类型
			else if(strcmp(type,"file")==0)
			{
				
				int ret;
				int file_sock;
				char filename[50];
				bzero(filename,50);
				
				//遍历寻找需要转发信息的客户端
				while(p != head)
				{
					if(p->sock == tmp&& (strcmp(p->ip,ip1)==0))
					{
						file_sock = p->sock;
						break;
					}
					p = p->next;
				}
				//等待发送方发来文件名字
				printf("等待接收文件名\n");
				read(newclient->sock,filename,sizeof(filename));
				
				sprintf(str, "%s:%s",type, filename);
				printf("str:%s\n",str);
				
				
				int filesize;
				
				/*****转发文件内容给指定的好友*****/
				//接收文件大小
				bzero(msg,sizeof(msg));
				read(newclient->sock,&filesize,4);
				
				//文件为空则退出循环
				if(filesize==0)
				{
					printf("文件为空！\n");
					break;
				}
				
				//文件不为空就分配一样的内存大小
				char *p = malloc(filesize);
				
				//向指定的客户端发送类型
				write(file_sock,str,strlen(str));
				
				
				printf("发送过来的文件大小是:%d\n",filesize);
				
				//接收整个文件的大小
				ret = recv(newclient->sock,p,filesize,MSG_WAITALL);
				
				printf("recv返回值:%d\n",ret);
				//发送整个文件的大小
				write(file_sock,p,filesize);
				printf("发送文件完成！\n");
				
			}
			//转发图片消息类型
			else if(strcmp(type,"pic")==0)
			{
				
				int ret;
				int file_sock;
				char filename[50];
				bzero(filename,50);
				while(p != head)
				{
					if(p->sock == tmp&& (strcmp(p->ip,ip1)==0))
					{
						file_sock = p->sock;
						break;
					}
					p = p->next;
				}
				printf("等待接收图片\n");
				read(newclient->sock,filename,sizeof(filename));
				
				sprintf(str, "%s:%s",type, filename);
				printf("str:%s\n",str);
				
				//接收文件大小
				int filesize;
				bzero(msg,sizeof(msg));
				read(newclient->sock,&filesize,4);
				
				//文件为空则退出循环
				if(filesize==0)
				{
					printf("文件为空！\n");
					break;
				}
				//向指定的客户端发送类型
				write(file_sock,str,strlen(str));
				//文件不为空就分配一样的内存大小
				char *p = malloc(filesize);
				
				printf("发送过来的图片大小是:%d\n",filesize);
				
				//接收整个文件的大小
				ret = recv(newclient->sock,p,filesize,MSG_WAITALL);
				
				printf("recv返回值:%d\n",ret);
				//发送整个文件的大小
				write(file_sock,p,filesize);
				printf("发送图片完成！\n");
				
				
			}
		}		

	}
}
}


//udp广播发送函数
int udp_broadcast(tcp_t head,int sock,char *ip)
{
	tcp_t p = head->next;
	
	
	int ret;
	char buf[50];
	
	struct sockaddr_in bindaddr;
	bzero(&bindaddr,sizeof(bindaddr));
	bindaddr.sin_family=PF_INET;
	bindaddr.sin_addr.s_addr=htonl(INADDR_ANY); //inet_pton() inet_aton()
	bindaddr.sin_port=htons(20000);
	
	struct sockaddr_in otheraddr;
	bzero(&otheraddr,sizeof(otheraddr));
	otheraddr.sin_family=PF_INET;
	otheraddr.sin_addr.s_addr=inet_addr("192.168.6.255"); //必须使用广播地址
	otheraddr.sin_port=htons(10088); //对方的端口号
	
	//创建udp套接字
	udpsock=socket(PF_INET,SOCK_DGRAM,0);
	if(udpsock==-1)
	{
		perror("创建udp套接字失败!\n");
		return -1;
	}
	
	//取消绑定限制
	int on=1;
	setsockopt(udpsock,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
	
	//绑定
	ret=bind(udpsock,(struct sockaddr *)&bindaddr,sizeof(bindaddr));
	if(ret==-1)
	{
		perror("绑定失败!\n");
		return -1;
	}
	
	//设置udp套接字可以广播
	setsockopt(udpsock, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));
	
	//发送信息

	bzero(buf,50);
	sprintf(buf,"ip:%s sock:%d的好友已上线!",ip,sock);

	sendto(udpsock,buf,strlen(buf),0,(struct sockaddr *)&otheraddr,sizeof(otheraddr));

		
}


//tcp监听客户端连接函数
int tcp_communication(tcp_t head)
{
	//创建套接字用于通信

	int res;
	//head = create_dlist();
	tcp_t new_client;
	tcpsocket = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(10001);

	
	//取消绑定限制
	int on=1;
	setsockopt(tcpsocket,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
	
	
	//绑定
	res = bind(tcpsocket, (struct sockaddr *)&server,sizeof(server));
	if(res == -1)
	{

		perror("bind tcp failed");
		return -1;
	}
	//监听
	listen(tcpsocket,10);

	struct sockaddr_in client;
	int len = sizeof(client);

	while(1)
	{
		//等待客户端连接
		int newsock = accept(tcpsocket, (struct sockaddr *)&client,&len);
		if(newsock==-1)
		{
			perror("accept failed");
			exit(0);
		}
		
		printf("ip:%s   端口：%d   sock:%d\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port),newsock);
	
		//将连上的客户端ip，sock插入链表
		new_client = (tcp_t)malloc(sizeof(tcp));
	
		new_client->sock = newsock;
		new_client->port = ntohs(client.sin_port);
		strcpy(new_client->ip,inet_ntoa(client.sin_addr));
		//新节点插入链表
		insert_node(head,&new_client);
		//udp广播客户端上线消息
		udp_broadcast(head,new_client->sock,new_client->ip);
		
		printf("广播成功\n");

		//为每个客户端创建线程来通信
		pthread_t tid;
		pthread_create(&tid,NULL,rdwr_client,new_client);
		pthread_detach(tid);
	
	}
}