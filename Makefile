CC = gcc
CFLAGS  = -Wall -g -O3
LIBS += -lm 

all: mirror_client writer.o reader.o exit.o

mirror_client:  main.o client.o
	$(CC) $(CFLAGS) -o mirror_client main.o client.o  $(LIBS)

client.o: client.c client.h
	$(CC) $(CFLAGS) -c client.c 

reader.o: reader.c reader.h
	$(CC) $(CFLAGS) -o reader.o reader.c 

writer.o: writer.c writer.h
	$(CC) $(CFLAGS) -o writer.o writer.c

exit.o: exit.c 
	$(CC) $(CFLAGS) -o exit.o exit.c

clean: 
	-rm -f *.o 
	-rm -f rhj