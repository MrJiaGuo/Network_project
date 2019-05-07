#ifndef _TCPUDP_H
#define _TCPUDP_H
#include "../dlist/dlist.h"
int udp_broadcast(tcp_t head,int sock,char *ip);

int tcp_communication(tcp_t head);




#endif