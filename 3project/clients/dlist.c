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
#include <sys/mman.h>
#include<semaphore.h>
#include <linux/input.h>
#include "dlist.h"
#include "aboutbmp.h"

char numpath[10][20]={
	"/haida/0.bmp",
	"/haida/1.bmp",
	"/haida/2.bmp",
	"/haida/3.bmp",
	"/haida/4.bmp",
	"/haida/5.bmp",
	"/haida/6.bmp",
	"/haida/7.bmp",
	"/haida/8.bmp",
	"/haida/9.bmp"
};

//触摸屏触控事件
struct input_event g_ts_event;

//映射内存
unsigned int *mem_p;

int fd_lcd,fd_ts;//文件描述符

int ts_x;
int ts_y;



//加载lcd驱动
int lcd_open()
{
	fd_lcd = open("/dev/fb0",O_RDWR);
    if(fd_lcd==-1){
        perror("open lcd");
        return -1;
    }
	
	mem_p = mmap(NULL,800*480*4,PROT_READ|PROT_WRITE,MAP_SHARED,fd_lcd,0);

    if(mem_p==MAP_FAILED){
        perror("mmap");
        return -1;
    }
	
}

//触摸事件驱动
int ts_open()
{
	fd_ts = open("/dev/input/event0",O_RDONLY);
	if(fd_ts  == -1)
	{
		perror(" Open ts");
		return -1;
	}
}


//获取触摸屏坐标
int	ts_xy_get(int *x,int *y)
{
	static int ts_get_xy_count=0;
	
	int count;
	
	while(1)
	{
		/* 调用read函数,获取触摸屏输入事件报告 */	
		count = read(fd_ts,&g_ts_event,sizeof(struct input_event));
		
		/* 检查当前读取的事件报告是否读取完整 */
		if(count != sizeof(struct input_event))
		{
			perror("read error");
			return -1;
		}	

		/* 检查当前响应事件是否坐标值事件 */
		if(EV_ABS == g_ts_event.type)
		{
			/* x坐标 */
			if(g_ts_event.code == ABS_X)
			{
				ts_get_xy_count ++;
				
				*x = g_ts_event.value;
				
			}
			
			/* y坐标 */
 			if(g_ts_event.code == ABS_Y)
			{
				ts_get_xy_count ++;
				
				*y = g_ts_event.value;	
							
			}	
			
			if(ts_get_xy_count == 2)
			{
				ts_get_xy_count = 0;
				break;
			}
		}
	}
	return 0;
}

tcp_t create_head()
{
	tcp_t newnode = (tcp_t)malloc(sizeof(tcp));
	if(!newnode)
	{
		perror("create failed");
		exit(0);
	}
	bzero(newnode->ip,sizeof(newnode->ip));
	newnode->port = -1;
	newnode->sock = -1;

	newnode->prev = newnode;
	newnode->next = newnode;

	return newnode;
	
}

tcp_t insert_node(tcp_t head,tcp_t *newnode1)
{
	tcp_t newnode = *newnode1;

	tcp_t p = head;
	newnode->next = head;
	newnode->prev = head->prev;
	head->prev->next = newnode;
	head->prev = newnode;

	return head;

}


tcp_t delete_node(tcp_t head, int sock)
{
	tcp_t p = head->next;
	while(p->sock != sock )
	{
		p = p->next;

	}
	
	tcp_t q = p;
	q->prev->next = q->next;
	q->next->prev = q->prev;
	free(p);
	q = NULL;

}


tcp_t travel_node(tcp_t head)
{
	tcp_t p = head->next;

    while(p!=head){
        printf("ip:%s   端口号：%d   sock: %d\n",p->ip,p->port,p->sock);
        p = p->next;
    }
    printf("\n");
	
}


tcp_t destroy_dlist(tcp_t *phead)
{
	tcp_t head = (*phead)->next,p=NULL;
	
	while(head!=*phead)
	{
		p=head;
		head=head->next;
		free(p);
		
	}
	free(head);
	*phead = NULL;
	p=NULL;
	
}

