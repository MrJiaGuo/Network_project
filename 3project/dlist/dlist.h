#ifndef _DLIST_H
#define _DLIST_H

typedef struct tcp_node{

	char ip[16];
	unsigned int port;
	int sock;
	struct tcp_node *prev;
	struct tcp_node *next;

}tcp,*tcp_t;


tcp_t create_head();
tcp_t destroy_dlist(tcp_t *phead);
tcp_t insert_node(tcp_t p, tcp_t *newnode1);
tcp_t travel_node(tcp_t head);

tcp_t delete_node(tcp_t head, int sock);




int lcd_open();
int ts_open();
int ts_close();

int	ts_xy_get(int *x,int *y);

#endif