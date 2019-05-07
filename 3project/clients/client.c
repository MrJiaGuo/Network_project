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
	客户端，通过服务器作为中间人，与其他的客户端进行通信！
	可与其他客户端进行点对点通信，发送消息，发送文件，发送表情！
	可与其他在线好友进行群聊！
	
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
};

//触屏文件描述符
extern int fd_ts;

//udp和tcp套接字
int udpsock;
int tcpsocket;

//相关数组，用来存放发送信息或接收发送信息
int ret;
char buf[100];
char sbuf[100];
char rbuf[50];

//存放文件名字的数组
char fi_buf[50];

//打开文件的文件描述符定义
int fd;

//标记位
int flag = 0;

//选择变量
int choice = 0;

//定义触摸屏坐标
int ts_x,ts_y;  


//显示在线好友函数
int show_friend()
{
	printf("hhhhhhh\n");
	char show_buf[100];

	int m;
	while(1)
	{
		bzero(show_buf,sizeof(show_buf));
		m = read(tcpsocket,show_buf,sizeof(show_buf));
		
		if(m<=1)
			return 0;
		else
		{
			printf("%s",show_buf);
		
			write(tcpsocket,"1",1);
		}
		
	}
	
}

//选择发送图片函数
int send_picture()
{
	int i ,j;
	//在开发板遍历显示图片
	for(i = 0;i<8;i++)
	{
		show_shapebmp(i*100,0,80,80,picpath[i]);
		usleep(200000);
	}
	
	//等待触屏
	ts_xy_get(&ts_x,&ts_y);
	
	//判断触屏坐标来获取图片
	for(j = 0;j<8;j++)
	{
		if(ts_x>j*100&&ts_x<((j+1)*100)&&ts_y>0&&ts_y<100)
		{
			strcpy(fi_buf,picpath[j]);
			return 0;
		}
			
	}
	//点到其他位置，默认发送表情1
	strcpy(fi_buf,picpath[0]);
	
}


//开发板触屏选择函数
int select_case()
{
	ts_xy_get(&ts_x,&ts_y);
	//1为点播消息
	if(ts_x>10&&ts_x<185&&ts_y>400&&ts_y<460)
		choice = 1;
	//2为文件发送
	else if(ts_x>220&&ts_x<390&&ts_y>400&&ts_y<460)
		choice = 2;
	//3为表情发送
	else if(ts_x>410&&ts_x<580&&ts_y>400&&ts_y<460)
		choice = 3;
	//4为广播群聊
	else if(ts_x>610&&ts_x<800&&ts_y>400&&ts_y<460)
		choice = 4;
	else
		choice = 0;
	
}

//发送文件函数
int send_file()
{

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

	//打开要发送的文件
	fd=open(fi_buf,O_RDWR|O_CREAT);
	
	if(fd == -1)
	{
		perror("open file failed");
		return -1;
	}
	
	//向服务器端发送文件名字
	write(tcpsocket,fi_buf,strlen(fi_buf));
	
	usleep(50000);
	
	//获取文件属性（大小）
	struct stat mystat;
	stat(fi_buf,&mystat);
	filesize=mystat.st_size;
	
	//向服务器端发送文件的大小
	write(tcpsocket,&filesize,4);

	//定义指针分配堆空间存放文件内容
	char *filenr=malloc(filesize);
	printf("文件大小是:%ld\n",mystat.st_size);
	
	//读取整个文件的大小
	read(fd,filenr,filesize);
	//整个文件大小全部发送给对方
	ret = write(tcpsocket,filenr,filesize);
	printf("send返回值:%d\n",ret);

	
	close(fd);
	sleep(2);
		
}



//选择发送消息的类型函数定义
int select_info()
{
	int num = 0;
	int i ;
	char com_buf[50];
	char *c = ":";
	printf("请在开发板上选择你需要的操作：\n");
	printf("**************************************************\n");
	printf("1.向好友发送信息，格式(info1:IP:SOCK)！\n");
	printf("2.向好友发送文件，格式(file:IP:SOCK)！\n");
	printf("3.向好友发送图片，格式(pic:IP:SOCK)！\n");
	printf("4.向所有在线好友发送信息，格式(broadcast)！\n");
	printf("**************************************************\n");
	
	//键盘控制选择类型
	/*while(scanf("%d",&choice)!=1)
	{       
		printf("输入错误，请重新输入:\n");
		scanf("%*[^\n]");
		scanf("%*c");
	}
	
	scanf("%*[^\n]");
	scanf("%*c");*/
	
	//触摸屏控制选择类型
	select_case();
	
	switch(choice)
	{
		case 1:
			bzero(sbuf,100);
			printf("请输入对方的地址：格式(info:IP:SOCK)\n");
			scanf("%s",sbuf);
			
			//控制输入的格式
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
			
			bzero(com_buf,50);
			strcpy(com_buf,sbuf);
			if(strcmp(strtok(com_buf,c),"info") != 0)
			{
				printf("输入格式错误，请重新选择\n");
				break;
			}
		
			//格式输入正确后发送服务器
			write(tcpsocket,sbuf,strlen(sbuf));
			printf("send ok\n");
			
			printf("请输入消息内容：\n");
			bzero(sbuf,100);
			scanf("%s",sbuf);
			send(tcpsocket,sbuf,strlen(sbuf),0);
			show_shapebmp(20,125,160,160,"/haida/infook.bmp");
			sleep(2);
			show_shapebmp(20,125,160,160,"/haida/ok.bmp");
			break;
		
		case 2:
			bzero(sbuf,100);
			printf("请输入对方的地址：格式(file:IP:SOCK)\n");
			scanf("%s",sbuf);
			
			//控制输入的格式
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
			
			bzero(com_buf,50);
			strcpy(com_buf,sbuf);
			if(strcmp(strtok(com_buf,c),"file") != 0)
			{
				printf("输入格式错误，请重新选择\n");
				break;
			}
			
			//格式输入正确后发送服务器
			write(tcpsocket,sbuf,strlen(sbuf));
			send_file();
			
			show_shapebmp(620,125,160,160,"/haida/fileok.bmp");
			sleep(2);
			show_shapebmp(620,125,160,160,"/haida/ok.bmp");
			break;
		
		case 3:
			flag = 1;
			bzero(sbuf,100);
			printf("请输入对方的地址：格式(pic:IP:SOCK)\n");
			scanf("%s",sbuf);
			
			//控制输入的格式
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
			
			bzero(com_buf,50);
			strcpy(com_buf,sbuf);
			if(strcmp(strtok(com_buf,c),"pic") != 0)
			{
				printf("输入格式错误，请重新选择\n");
				break;
			}
			
			//格式输入正确后发送服务器
			write(tcpsocket,sbuf,strlen(sbuf));
			
			send_file();
			break;
			
		case 4:
			
			bzero(sbuf,100);
			printf("请输入广播格式(broadcast)！\n");
			scanf("%s",sbuf);
			
			//控制输入的格式
			if(strcmp(sbuf,"broadcast")!=0)
			{
				printf("输入格式错误，请重新选择\n");	
				break;	
			}
			
			//格式输入正确后发送服务器
			write(tcpsocket,sbuf,strlen(sbuf));
			
			printf("请输入广播消息内容：\n");
			bzero(sbuf,100);
			scanf("%s",sbuf);
			send(tcpsocket,sbuf,strlen(sbuf),0);
			
			show_shapebmp(20,125,160,160,"/haida/infook.bmp");
			sleep(2);
			show_shapebmp(20,125,160,160,"/haida/ok.bmp");
			break;
		
		default:
			printf("选择选项不存在，请重新选择!\n");
		
	}
	
}


//tcp连接函数，连接上服务器端
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

}

//线程函数一直调用选择函数来控制选择消息发送类型
void *func(void *arg)
{
	while(1)
	{
		select_info();
	}
	
}


int main(int argc,char **argv)
{
	//lcd和触摸屏驱动打开
	lcd_open();
	ts_open();
	
	//触摸屏显示主界面
	show_shapebmp(0,0,800,480,"/haida/mainwindow.bmp");
	show_shapebmp(20,125,160,160,"/haida/ok.bmp");
	show_shapebmp(620,125,160,160,"/haida/ok.bmp");
	
	
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
	
	//调用tcp客户端连接函数
	tcp_client();
	
	//创建线程
	pthread_t tid;
	pthread_create(&tid,NULL,func,NULL);
	pthread_detach(tid);
	
	//设置多路复用监控udp和tcp套接字
	fd_set myset;

	int max_sock = udpsock;
	if(max_sock<tcpsocket)
		max_sock = tcpsocket;
	if(max_sock<fd_ts)
		max_sock = fd_ts;
	
	//接收信息
	while(1)
	{
		//添加文件描述符，select会将状态没有改变的文件描述符从集合中剔出
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
		
		/*
		//判断键盘是否发生了读就绪
		if(FD_ISSET(0,&myset))//键盘在集合中就说明键盘发生了读就绪
		{
					select_info();
		}
		
		if(FD_ISSET(fd_ts,&myset))//触摸屏在集合中就说明键盘发生了读就绪
		{
			select_info();
		}
		*/
		
		//判断tcp是否发生了读就绪
		if(FD_ISSET(tcpsocket,&myset))
		{
			bzero(buf,100);
			ret = read(tcpsocket,buf,sizeof(buf));
			if(ret == 0)
			{
				printf("服务器已断开!\n");
				return -1;
			}
			char filename[50];
			char type[50];
			
			bzero(filename,50);
			bzero(type,50);
			
			//判断接收消息类型是否是广播消息
			if(strcmp(buf,"broadcast")==0)
			{
				bzero(buf,100);
				ret = read(tcpsocket,buf,sizeof(buf));
				if(ret == 0)
				{
					printf("服务器已断开!\n");
					exit(-1);
				}
			
				printf("%s\n",buf);
				show_shapebmp(20,125,160,160,"/haida/recvinfo.bmp");
				sleep(1);
				show_shapebmp(20,125,160,160,"/haida/ok.bmp");
				
			}
			else
			{
				//拆分消息
				char *c = ":";
				strcpy(type,strtok(buf,c));
				strcpy(filename,strtok(NULL,c));

				//判断接收消息类型是否是单播消息
				if(strcmp(type,"info")==0)
				{
					
					bzero(buf,100);
					ret = read(tcpsocket,buf,sizeof(buf));
					if(ret == 0)
					{
						perror("服务器已断开");
						return -1;
					}
				
					printf("%s\n",buf);
					show_shapebmp(20,125,160,160,"/haida/recvinfo.bmp");
					sleep(2);
					show_shapebmp(20,125,160,160,"/haida/ok.bmp");
					
				}
				//判断接收消息类型是否是文件消息
				else if(strcmp(type,"file")==0)
				{
					int fd2 = open(filename,O_WRONLY|O_CREAT|O_TRUNC,0777);
					if(fd2==-1)
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
							printf("服务器已断开!\n");
							return -1;
						}
						write(fd2,buf,ret);
						
						if(ret<100)
						{
							printf("接收文件完成！\n");
							show_shapebmp(20,125,160,160,"/haida/recvfile.bmp");
							sleep(2);
							show_shapebmp(20,125,160,160,"/haida/ok.bmp");				
							close(fd2);
							break;
						}
						
					}
					
				}
				//判断接收消息类型是否是图片消息
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
							printf("服务器已断开!\n");
							return -1;
						}
						write(fd1,buf,ret);
						
						if(ret<100)
						{
							printf("接收图片完成！\n");												
							usleep(500000);
							close(fd1);
							usleep(500000);
							show_shapebmp(365,210,80, 80,filename);
							break;
						}
						
					}
					
					
					
				}
			}
			
		}
		
		//判断套接字是否发生了读就绪
		if(FD_ISSET(udpsock,&myset))//udpsock在集合中就说明udpsock发生了读就绪
		{

			bzero(rbuf,50);
			ret=recvfrom(udpsock,rbuf,50,0,(struct sockaddr *)&otheraddr,&addrsize);
			printf("广播信息的ip是:%s  端口是:%hu \n",inet_ntoa(otheraddr.sin_addr),ntohs(otheraddr.sin_port));
			printf("接收的广播内容是:%s\n",rbuf);
			
		}
		
	}
	
	//关闭套接字
	close(udpsock);
	close(tcpsocket);
	return 0;
}