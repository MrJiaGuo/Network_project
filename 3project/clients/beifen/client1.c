#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include "dlist.h"
#include "aboutbmp.h"
/*
	udp广播通信流程
	
*/
char picpath[8][20]={
	"/haida/p0.bmp",
	"/haida/p1.bmp",
	"/haida/p2.bmp",
	"/haida/p3.bmp",
	"/haida/p4.bmp",
	"/haida/p5.bmp",
	"/haida/p6.bmp",
	"/haida/p7.bmp"
	//"/haida/p8.bmp",
};


extern int fd_ts;

int udpsock;
int tcpsocket;

int ret;
char buf[100];
char sbuf[100];
char rbuf[50];

//存放文件名字的数组
char fi_buf[50];

int fd;

int flag = 0;

int choice = 0;

//定义触摸屏坐标
int ts_x,ts_y;  

int show_friend()
{
	char show_buf[100];

	int m;
	while(1)
	{
		bzero(show_buf,sizeof(show_buf));
		m = read(tcpsocket,show_buf,sizeof(show_buf));
		
		//printf("m:%d\n",m);
		if(m<=1)
			return 0;
		else
			printf("%s",show_buf);
			
	}
	
}


int send_picture()
{
	int i ,j;
	for(i = 0;i<9;i++)
	{
		show_shapebmp(i*100,0,80,80,picpath[i]);
		usleep(500000);
	}
	
	ts_xy_get(&ts_x,&ts_y);
	
	for(j = 0;j<8;j++)
	{
		if(ts_x>j*100&&ts_x<((j+1)*100)&&ts_y>0&&ts_y<100)
		{
			strcpy(fi_buf,picpath[j]);
		}
			
	}
	
}


int select_case()
{
	ts_xy_get(&ts_x,&ts_y);
	
	if(ts_x>35&&ts_x<240&&ts_y>370&&ts_y<460)
		choice = 1;
	else if(ts_x>310&&ts_x<510&&ts_y>370&&ts_y<460)
		choice = 2;
	else if(ts_x>560&&ts_x<800&&ts_y>370&&ts_y<460)
		choice = 3;
	else
		choice = 0;
	
}

int send_file()
{
	//文件名数组
	
	bzero(fi_buf,50);
	//文件大小是
	int filesize;
	//输入文件名
	if(flag == 0 )
	{
		printf("请输入文件名：\n");
		
		scanf("%s",fi_buf);
		printf("fi_buf:%s\n",fi_buf);
	}
	else
	{
		printf("请在开发板上选择需要发送的表情：\n");
		send_picture();
		flag = 0;
	}
	//scanf("%s",fi_buf);
	//printf("fi_buf:%s",fi_buf);
	write(tcpsocket,fi_buf,strlen(fi_buf));
	
	//usleep(50000);
	
	struct stat mystat;
	//打开要发送的文件
	fd=open(fi_buf,O_RDWR);
	
	if(fd == -1)
	{
		perror("open file failed");
		exit(0);
	}
	
	stat(fi_buf,&mystat);
	filesize=mystat.st_size;
	//定义指针分配堆空间存放文件内容
	char *filenr=malloc(filesize);
	printf("文件大小是:%ld\n",mystat.st_size);
	
	//整个文件大小全部发送给对方
	read(fd,filenr,filesize);
	ret = send(tcpsocket,filenr,filesize,0);
	printf("send返回值:%d\n",ret);
	
	close(fd);
}


int select_info()
{
	int num = 0;
	int i ;
	
	printf("请在开发板上选择你需要的操作：\n");
	printf("**************************************************\n");
	printf("1.向好友发送信息，格式(info:IP:SOCK)！\n");
	printf("2.向好友发送文件，格式(file:IP:SOCK)！\n");
	printf("3.向好友发送图片，格式(pic:IP:SOCK)！\n");
	printf("**************************************************\n");
	
	while(scanf("%d",&choice)!=1)
	{       
		printf("输入错误，请重新输入:\n");
		scanf("%*[^\n]");
		scanf("%*c");
	}
	
	scanf("%*[^\n]");
	scanf("%*c");
	
	//select_case();
	
	switch(choice)
	{
		case 1:
			bzero(sbuf,100);
			//获取键盘输入的内容
			printf("请输入对方的地址：格式(info:IP:SOCK)\n");
			scanf("%s",sbuf);
			
			/*if(sbuf[0] == 0)
			{
				printf("输入格式错误，请重新输入\n");
				goto lable1;	
			}*/
			
			for(i = 0;i<=strlen(sbuf);i++)
			{
				if(sbuf[i]==':')
					num++;
			}
			
			if(num<2||num>2)
			{
				printf("输入格式错误，请重新选择\n");
				num = 0;
				break;
			}
			
			send(tcpsocket,sbuf,strlen(sbuf),0);
			
			//判断是否存在该好友，等待服务器的回复
			bzero(sbuf,sizeof(sbuf));
			
			read(tcpsocket,sbuf,sizeof(sbuf));
			
			printf("sbuf:%s \n",sbuf);
			
			if(strcmp(sbuf,"notfind")==0)
			{
				printf("该好友不存在！\n");
				
				printf("当前在线好友如下！\n");
				show_friend();
				printf("exit ok\n");
				break;
			}
			
			printf("请输入消息内容：\n");
			bzero(sbuf,100);
			scanf("%s",sbuf);
			send(tcpsocket,sbuf,strlen(sbuf),0);
			break;
		
		case 2:
			bzero(sbuf,100);
			//获取键盘输入的内容
			printf("请输入对方的地址：格式(file:IP:SOCK)\n");
			scanf("%s",sbuf);
			
			for(i = 0;i<=strlen(sbuf);i++)
			{
				if(sbuf[i]==':')
					num++;
			}
			
			if(num<2||num>2)
			{
				printf("输入格式错误，请重新选择\n");
				num = 0;
				break;
			}
			
			send(tcpsocket,sbuf,strlen(sbuf),0);
			
			//判断是否存在该好友，等待服务器的回复
			bzero(sbuf,sizeof(sbuf));
			
			read(tcpsocket,sbuf,sizeof(sbuf));
			printf("sbuf:%s \n",sbuf);
			if(strcmp(sbuf,"notfind")==0)
			{
				printf("该好友不存在！\n");
				printf("当前在线好友如下！\n");
				show_friend();
				printf("exit ok\n");
				break;
			}
			
			
			send_file();
			break;
		
		case 3:
			flag = 1;
			bzero(sbuf,100);
			//获取键盘输入的内容
			printf("请输入对方的地址：格式(pic:IP:SOCK)\n");
			scanf("%s",sbuf);
			
			for(i = 0;i<=strlen(sbuf);i++)
			{
				if(sbuf[i]==':')
					num++;
			}
			
			if(num<2||num>2)
			{
				printf("输入格式错误，请重新选择\n");
				num = 0;
				break;
			}
			
			send(tcpsocket,sbuf,strlen(sbuf),0);
			
			//判断是否存在该好友，等待服务器的回复
			bzero(sbuf,sizeof(sbuf));
			
			read(tcpsocket,sbuf,sizeof(sbuf));
			printf("%s\n",sbuf);
			
			if(strcmp(sbuf,"bufisempty")==0)
			{
				printf("内容为空！\n");
				break;
			}
			
			if(strcmp(sbuf,"notfind")==0)
			{
				printf("该好友不存在！\n");
				printf("当前在线好友如下！\n");
				show_friend();
				printf("exit ok\n");
				break;
			}
			send_file();
			break;
		
		default:
			printf("选择选项不存在，请重新选择!\n");
		
	}
	
}



int tcp_client()
{
	
	//创建套接字用于通信
	tcpsocket = socket(AF_INET, SOCK_STREAM, 0);
	if(tcpsocket == -1)
	{
		perror("socket failed");
		exit(0);
	}
	//绑定

	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr("192.168.6.134");
	server.sin_port = htons(10001);

	//连接
	int res;
	res = connect(tcpsocket,(struct sockaddr *)&server,sizeof(server));

	if(res ==-1)
	{
		perror("connect failed");
		exit(0);
	}

	/*while(1)
	{
		bzero(buf,sizeof(buf));
		read(tcpsocket,buf,sizeof(buf));
		printf("广播信息为：%s\n",buf);
	}*/
	//发送或者接收数据

	//挂机

	
	
}


int main(int argc,char **argv)
{
	//lcd_open();
	//ts_open();
	
	//show_shapebmp(0,0,800,480,"/haida/mainwindow.bmp");
	

	struct sockaddr_in bindaddr;
	bzero(&bindaddr,sizeof(bindaddr));
	bindaddr.sin_family=PF_INET;
	bindaddr.sin_addr.s_addr=htonl(INADDR_ANY); //inet_pton() inet_aton()
	bindaddr.sin_port=htons(10088);
	
	struct sockaddr_in otheraddr;
	bzero(&otheraddr,sizeof(otheraddr));
	int addrsize=sizeof(otheraddr);
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
	
	tcp_client();
	
	
	fd_set myset;
	
	int max_sock = udpsock;
	
	if(max_sock<tcpsocket)
		max_sock = tcpsocket;
	if(max_sock<fd_ts)
		max_sock = fd_ts;
	
	//接收信息
	while(1)
	{
		
		
		//一定要放在循环里面，原因是select会将状态没有改变的文件描述符从集合中剔出
		FD_ZERO(&myset);
		FD_SET(0,&myset);
		//FD_SET(fd_ts,&myset);
		FD_SET(udpsock,&myset);
		FD_SET(tcpsocket,&myset);
		
		ret=select(max_sock+1,&myset,NULL,NULL,NULL);
		if(ret==-1)
		{
			perror("select调用失败!\n");
			return -1;
		}
		
		
		//判断键盘是否发生了读就绪
		if(FD_ISSET(0,&myset))//键盘在集合中就说明键盘发生了读就绪
		{
			select_info();
		}
		
		/*if(FD_ISSET(fd_ts,&myset))//触摸屏在集合中就说明键盘发生了读就绪
		{
			select_info();
		}*/
		
		
		//判断tcp是否发生了读就绪
		if(FD_ISSET(tcpsocket,&myset))//键盘在集合中就说明键盘发生了读就绪
		{
			bzero(buf,100);
			//获取键盘输入的内容
			ret = read(tcpsocket,buf,sizeof(buf));
			if(ret == 0)
			{
				perror("服务器已断开");
				return -1;
			}
			char filename[50];
			char type[50];
			
			bzero(filename,50);
			bzero(type,50);
			
			//printf("buf:%s\n",buf);
			
			char *c = ":";
			strcpy(type,strtok(buf,c));
			strcpy(filename,strtok(NULL,c));
			//strcpy(sock1,strtok(NULL,c));
			
			//printf("type :%s  filename:%s\n",type,filename);
			
			if(strcmp(type,"info")==0)
			{
				
				bzero(buf,100);
				//获取键盘输入的内容
				ret = read(tcpsocket,buf,sizeof(buf));
				if(ret == 0)
				{
					perror("服务器已断开");
					return -1;
				}
			
				printf("%s\n",buf);
				
			}
			
			else if(strcmp(type,"file")==0)
			{
				int fd = open(filename,O_WRONLY|O_CREAT|O_TRUNC,0666);
				if(fd==-1)
				{
					perror("open failed");
					exit(-3);
				}
				while(1)
				{
					bzero(buf,100);
					ret = read(tcpsocket,buf,sizeof(buf));
					if(ret == 0)
					{
						perror("服务器已断开");
						return -1;
					}
					write(fd,buf,ret);
					
					if(ret<100)
					{
						printf("接收文件完成！\n");
						sleep(1);
						close(fd);
						break;
					}
					
				}
				
			}
			else if(strcmp(type,"pic")==0)
			{
				int fd1 = open(filename,O_WRONLY|O_CREAT|O_TRUNC,0666);
				if(fd==-1)
				{
					perror("open failed");
					exit(-3);
				}
				while(1)
				{
					bzero(buf,100);
					ret = read(tcpsocket,buf,sizeof(buf));
					if(ret == 0)
					{
						perror("服务器已断开");
						return -1;
					}
					write(fd1,buf,ret);
					
					if(ret<100)
					{
						printf("接收图片完成！\n");
						
						
						sleep(1);
						close(fd1);
						usleep(500000);
						//show_shapebmp(400,200,80, 80,filename);
						break;
					}
					
				}
				
				
				
			}
			
			
		}
		
		
		
		//判断套接字是否发生了读就绪
		if(FD_ISSET(udpsock,&myset))//tcpsock在集合中就说明tcpsock发生了读就绪
		{

			bzero(rbuf,50);
			ret=recvfrom(udpsock,rbuf,50,0,(struct sockaddr *)&otheraddr,&addrsize);
			printf("广播信息的ip是:%s  端口是:%hu \n",inet_ntoa(otheraddr.sin_addr),ntohs(otheraddr.sin_port));
			printf("接收的广播内容是:%s\n",rbuf);
			
		}
		
	}
	
	//关闭
	close(udpsock);
	return 0;
}