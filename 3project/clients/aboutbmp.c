/*
	关于24位bmp图片显示的所有函数都在这里
	显示全屏
	任意位置显示任意大小
	将任意大小的bmp图片缩写任意比例
*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include "aboutbmp.h"

#include "dlist.h"
// 自定义函数，专门用于显示800*480bmp格式的图片
int show_fullbmp(char *bmpname)   //printf
{
	int i;
	int x,y;
	int lcdfd;
	int bmpfd;
	// 定义一个数组用于存放bmp的颜色值
	char bmpbuf[800*480*3];  // char占一个字节    bmpbuf[0] bmpbuf[1] bmpbuf[2]一组
	int lcdbuf[800*480]; // int占4字节
	
	// 定义一个临时变量保存颜色值
	int tempbuf[800*480];
	// 打开lcd的驱动
	lcdfd = open("/dev/fb0",O_RDWR);
	// 出错处理
	if(lcdfd == -1)
	{
		printf("open lcd error!\n");
		return -1;
	}
	// 打开你要显示的bmp图片
	bmpfd = open(bmpname,O_RDONLY);
	if(bmpfd == -1)
	{
		printf("open bmp %s error!\n",bmpname);
		return -1;
	}
	// 在读之前，先跳过前面54字节
	lseek(bmpfd,54,SEEK_SET);
	// 读取bmp图片的颜色值,从55字节位置开始读取了
	read(bmpfd,bmpbuf,800*480*3);
	
	// 将读取的颜色值3字节---》转换成4字节
	for(i=0; i<800*480; i++)
		lcdbuf[i]=bmpbuf[3*i]|bmpbuf[3*i+1]<<8|bmpbuf[3*i+2]<<16|0x00<<24;

	// 将颠倒的图片翻转过来(x,y)--->(x,479-y)
	for(x=0; x<800; x++)
		for(y=0; y<480; y++)
			//lcdbuf[(479-y)*800+x] = lcdbuf[y*800+x];
			tempbuf[(479-y)*800+x] = lcdbuf[y*800+x];
			
	
	//将转换之后的颜色值写入到lcd的驱动中
	write(lcdfd,tempbuf,800*480*4);
	
	// 关闭lcd和bmp
	close(bmpfd);
	close(lcdfd);
	return 0;
}

// 自定义函数，任意位置显示任意大小的bmp
int show_shapebmp(int x,int y,/*int w,int h*/int w, int h,char *bmpname)   //printf
{
	int i,j;
	int lcdfd;
	int bmpfd;
	int *lcdmem;
	// 定义一个数组用于存放bmp的颜色值
	char bmpbuf[w*h*3];  // char占一个字节    bmpbuf[0] bmpbuf[1] bmpbuf[2]一组
	int lcdbuf[w*h]; // int占4字节
	
	// 打开lcd的驱动
	lcdfd = open("/dev/fb0",O_RDWR);
	// 出错处理
	if(lcdfd == -1)
	{
		printf("open lcd error!\n");
		return -1;
	}

	// 将lcd的地址映射到用户空间
	lcdmem = (int *)mmap(NULL,800*480*4,PROT_READ|PROT_WRITE,MAP_SHARED,lcdfd,0);
	if(lcdmem==NULL)
	{
		printf("map lcd error!\n");
		return -1;
	}
	// 打开你要显示的bmp图片
	bmpfd = open(bmpname,O_RDONLY);
	if(bmpfd == -1)
	{
		printf("open bmp %s error!\n",bmpname);
		return -1;
	}
	// 在读之前，先跳过前面54字节
	lseek(bmpfd,54,SEEK_SET);
	// 读取bmp图片的颜色值,从55字节位置开始读取了
	read(bmpfd,bmpbuf,w*h*3);
	
	// 将读取的颜色值3字节---》转换成4字节
	for(i=0; i<w*h; i++)
		lcdbuf[i]=bmpbuf[3*i]|bmpbuf[3*i+1]<<8|bmpbuf[3*i+2]<<16|0x00<<24;

	// 将（i,j）---》存放在屏幕（i+x,j+y）的位置
	for(i=0; i<w; i++)
		for(j=0; j<h; j++)
			*(lcdmem+(j+y)*800+i+x) = lcdbuf[(h-1-j)*w+i];
	// 关闭lcd和bmp
	close(bmpfd);
	close(lcdfd);
	// 解除内存映射
	munmap(lcdmem,800*480*4);
	return 0;
	
	/*int i, j;
	
	if(x+bmp_w > 800 || y+bmp_h > 480)
	{
		printf("BMP too big!\n");

	}

	//1.打开bmp图片
	int bmp_fd = open(bmp_name, O_RDONLY);
	if(bmp_fd == -1)
	{
		perror("open 1.bmp faield!");
		return -1;
	}
	unsigned char bmp_buf[bmp_w*bmp_h*3];
	bzero(bmp_buf, bmp_w*bmp_h*3);
	
	//跳过54个字节文件头
	lseek(bmp_fd, 54, SEEK_SET);
	
	//2.读取bmp图片里面的数据
	read(bmp_fd, bmp_buf, bmp_w*bmp_h*3);
	
	//3.关闭bmp图片	
	close(bmp_fd);
	
	unsigned int lcd_buf[bmp_w*bmp_h];  //每个元素是四个字节
	bzero(lcd_buf, bmp_w*bmp_h*4);
	
	for(j=0; j<bmp_h; j++)
	{
		for(i=0; i<bmp_w; i++)
		{
			lcd_buf[i+j*bmp_w] = bmp_buf[3*(i+j*bmp_w)+0] | (bmp_buf[3*(i+j*bmp_w)+1]<<8) | (bmp_buf[3*(i+j*bmp_w)+2]<<16);
		}
		
	}	
	//5.打开lcd屏幕文件
	int lcd_fd = open("/dev/fb0", O_RDWR);
	if(lcd_fd == -1)
	{
		perror("open fb0 faield!");
		return -1;
	}
	//6.映射内存
	unsigned int *mmap_fd = mmap(NULL, 800*480*4,  PROT_READ | PROT_WRITE, MAP_SHARED, lcd_fd, 0);
	if(mmap_fd == (void*)-1)
	{
		perror("mmap failed!");
		return -1;
	}

	for(j=0; j<bmp_h; j++)
	{
		for(i=0; i<bmp_w; i++)
		{
			*(mmap_fd+i+(bmp_h-1-j)*800+x+y*800) = lcd_buf[i+j*bmp_w];		//非原点、倒立的
		}
	}
	//8.关闭lcd屏幕文件
	close(lcd_fd);
		
	return 0;*/	
}

