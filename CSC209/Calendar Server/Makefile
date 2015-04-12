PORT=53926
CFLAGS= -DPORT=\$(PORT) -g -Wall


calserver: calserver.o lists.o 
	gcc $(CFLAGS) -o calserver calserver.o lists.o

calserver.o: calserver.c lists.h
	gcc $(CFLAGS) -c calserver.c

lists.o: lists.c lists.h
	gcc $(CFLAGS) -c lists.c

clean: 
	rm calserver *.o