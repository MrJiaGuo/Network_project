CC=gcc
ELF=project
SRC=${wildcard *.c tcpudp/*.c bmp/*.c dlist/*.c}
OBJS=${patsubst %.c,%.o,${SRC}}
CFLAGS=-c
LDFLAGS=
RM=rm -rf

${ELF}:${OBJS}
	${CC} ${LDFLAGS} $^ -pthread -o $@
%.o:%.c
	${CC} ${CFLAGS} $< -o $@
.PHONY:clean
clean:
	${RM} ${OBJS} ${ELF}


#wildcard：扩展通配符，把指定目录下的所有后缀是c的文件全部展开
#patsubst：替换通配符，把wildcard展开的文件中以.c为后缀的全部替换成.o
#notdir：把展开的文件去掉路径信息
#.PHONY:伪造的，假的，后接的clean是伪造的，不是真正存在的目标，因为当真正存在clean文件的时候，不加这个会提示错误
#$@：目标文件
#$^：所有的依赖文件
#$<：第一个依赖文件


#project:main.o tcpudp.o dlist.o aboutbmp.o
#	gcc main.o tcpudp.o dlist.o aboutbmp.o -pthread -o project
#main.o:main.c
#	gcc -c main.c
#init.o:tcpudp.c
#	gcc -c tcpudp.c
#dlist.o:dlist.c
#	gcc -c dlist.c
#aboutbmp.o:aboutbmp.c
#	gcc -c aboutbmp.c

#clean:
#	rm -r *.o project