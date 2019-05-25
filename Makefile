
# Specify the compiler
CC = g++

#CCOPTS = -ansi -pedantic -Wall -g
CCOPTS = -g -Wall
LIBS = -lpthread

# Make the source
all:	sender receiver

common.o : common.h common.cpp 
	$(CC) $(CCOPTS) -c common.cpp
 
sender:  sender1.cpp common.o  
	$(CC) $(CCOPTS) $(LIBS) common.o sender1.cpp -o sender

receiver: receiver1.cpp common.o
	$(CC) $(CCOPTS) $(LIBS) common.o receiver1.cpp -o receiver

clean :
	rm -f common.o sender receiver
