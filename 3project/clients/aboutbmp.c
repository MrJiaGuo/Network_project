/*
	����24λbmpͼƬ��ʾ�����к�����������
	��ʾȫ��
	����λ����ʾ�����С
	�������С��bmpͼƬ��д�������
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
// �Զ��庯����ר��������ʾ800*480bmp��ʽ��ͼƬ
int show_fullbmp(char *bmpname)   //printf
{
	int i;
	int x,y;
	int lcdfd;
	int bmpfd;
	// ����һ���������ڴ��bmp����ɫֵ
	char bmpbuf[800*480*3];  // charռһ���ֽ�    bmpbuf[0] bmpbuf[1] bmpbuf[2]һ��
	int lcdbuf[800*480]; // intռ4�ֽ�
	
	// ����һ����ʱ����������ɫֵ
	int tempbuf[800*480];
	// ��lcd������
	lcdfd = open("/dev/fb0",O_RDWR);
	// ������
	if(lcdfd == -1)
	{
		printf("open lcd error!\n");
		return -1;
	}
	// ����Ҫ��ʾ��bmpͼƬ
	bmpfd = open(bmpname,O_RDONLY);
	if(bmpfd == -1)
	{
		printf("open bmp %s error!\n",bmpname);
		return -1;
	}
	// �ڶ�֮ǰ��������ǰ��54�ֽ�
	lseek(bmpfd,54,SEEK_SET);
	// ��ȡbmpͼƬ����ɫֵ,��55�ֽ�λ�ÿ�ʼ��ȡ��
	read(bmpfd,bmpbuf,800*480*3);
	
	// ����ȡ����ɫֵ3�ֽ�---��ת����4�ֽ�
	for(i=0; i<800*480; i++)
		lcdbuf[i]=bmpbuf[3*i]|bmpbuf[3*i+1]<<8|bmpbuf[3*i+2]<<16|0x00<<24;

	// ���ߵ���ͼƬ��ת����(x,y)--->(x,479-y)
	for(x=0; x<800; x++)
		for(y=0; y<480; y++)
			//lcdbuf[(479-y)*800+x] = lcdbuf[y*800+x];
			tempbuf[(479-y)*800+x] = lcdbuf[y*800+x];
			
	
	//��ת��֮�����ɫֵд�뵽lcd��������
	write(lcdfd,tempbuf,800*480*4);
	
	// �ر�lcd��bmp
	close(bmpfd);
	close(lcdfd);
	return 0;
}

// �Զ��庯��������λ����ʾ�����С��bmp
int show_shapebmp(int x,int y,/*int w,int h*/int w, int h,char *bmpname)   //printf
{
	int i,j;
	int lcdfd;
	int bmpfd;
	int *lcdmem;
	// ����һ���������ڴ��bmp����ɫֵ
	char bmpbuf[w*h*3];  // charռһ���ֽ�    bmpbuf[0] bmpbuf[1] bmpbuf[2]һ��
	int lcdbuf[w*h]; // intռ4�ֽ�
	
	// ��lcd������
	lcdfd = open("/dev/fb0",O_RDWR);
	// ������
	if(lcdfd == -1)
	{
		printf("open lcd error!\n");
		return -1;
	}

	// ��lcd�ĵ�ַӳ�䵽�û��ռ�
	lcdmem = (int *)mmap(NULL,800*480*4,PROT_READ|PROT_WRITE,MAP_SHARED,lcdfd,0);
	if(lcdmem==NULL)
	{
		printf("map lcd error!\n");
		return -1;
	}
	// ����Ҫ��ʾ��bmpͼƬ
	bmpfd = open(bmpname,O_RDONLY);
	if(bmpfd == -1)
	{
		printf("open bmp %s error!\n",bmpname);
		return -1;
	}
	// �ڶ�֮ǰ��������ǰ��54�ֽ�
	lseek(bmpfd,54,SEEK_SET);
	// ��ȡbmpͼƬ����ɫֵ,��55�ֽ�λ�ÿ�ʼ��ȡ��
	read(bmpfd,bmpbuf,w*h*3);
	
	// ����ȡ����ɫֵ3�ֽ�---��ת����4�ֽ�
	for(i=0; i<w*h; i++)
		lcdbuf[i]=bmpbuf[3*i]|bmpbuf[3*i+1]<<8|bmpbuf[3*i+2]<<16|0x00<<24;

	// ����i,j��---���������Ļ��i+x,j+y����λ��
	for(i=0; i<w; i++)
		for(j=0; j<h; j++)
			*(lcdmem+(j+y)*800+i+x) = lcdbuf[(h-1-j)*w+i];
	// �ر�lcd��bmp
	close(bmpfd);
	close(lcdfd);
	// ����ڴ�ӳ��
	munmap(lcdmem,800*480*4);
	return 0;
	
	/*int i, j;
	
	if(x+bmp_w > 800 || y+bmp_h > 480)
	{
		printf("BMP too big!\n");

	}

	//1.��bmpͼƬ
	int bmp_fd = open(bmp_name, O_RDONLY);
	if(bmp_fd == -1)
	{
		perror("open 1.bmp faield!");
		return -1;
	}
	unsigned char bmp_buf[bmp_w*bmp_h*3];
	bzero(bmp_buf, bmp_w*bmp_h*3);
	
	//����54���ֽ��ļ�ͷ
	lseek(bmp_fd, 54, SEEK_SET);
	
	//2.��ȡbmpͼƬ���������
	read(bmp_fd, bmp_buf, bmp_w*bmp_h*3);
	
	//3.�ر�bmpͼƬ	
	close(bmp_fd);
	
	unsigned int lcd_buf[bmp_w*bmp_h];  //ÿ��Ԫ�����ĸ��ֽ�
	bzero(lcd_buf, bmp_w*bmp_h*4);
	
	for(j=0; j<bmp_h; j++)
	{
		for(i=0; i<bmp_w; i++)
		{
			lcd_buf[i+j*bmp_w] = bmp_buf[3*(i+j*bmp_w)+0] | (bmp_buf[3*(i+j*bmp_w)+1]<<8) | (bmp_buf[3*(i+j*bmp_w)+2]<<16);
		}
		
	}	
	//5.��lcd��Ļ�ļ�
	int lcd_fd = open("/dev/fb0", O_RDWR);
	if(lcd_fd == -1)
	{
		perror("open fb0 faield!");
		return -1;
	}
	//6.ӳ���ڴ�
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
			*(mmap_fd+i+(bmp_h-1-j)*800+x+y*800) = lcd_buf[i+j*bmp_w];		//��ԭ�㡢������
		}
	}
	//8.�ر�lcd��Ļ�ļ�
	close(lcd_fd);
		
	return 0;*/	
}

