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
#include "tcpudp/tcpudp.h"
#include "bmp/aboutbmp.h"
#include "dlist/dlist.h"

tcp_t head;

int main()
{

	
	head = create_head();
	
	tcp_communication(head);
	
	
	
	return 0;
	
}