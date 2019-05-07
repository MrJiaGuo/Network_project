project:client.o dlist.o aboutbmp.o
	arm-linux-gcc client.o dlist.o aboutbmp.o -pthread -o client
client.o:client.c
	arm-linux-gcc -c client.c
dlist.o:dlist.c
	arm-linux-gcc -c dlist.c
aboutbmp.o:aboutbmp.c
	arm-linux-gcc -c aboutbmp.c

clean:
	rm -r *.o client
