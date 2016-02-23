#-------------------------------------------------------------------------------
# Project 1
# Written by  : Daniel Lynch
# Last Modified : 01/28/2016
#-------------------------------------------------------------------------------

.PHONY: clean all tarball

all: sockets

clean:
	rm -f consumer producer server
	rm -f *.o

# These targets take care of the compilation process.
sockets: 
	gcc -c server.c
	gcc -o server server.c -lpthread
	gcc -c producer.c
	gcc -o producer producer.c
	gcc -c consumer.c
	gcc -o consumer consumer.c