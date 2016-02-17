#-------------------------------------------------------------------------------
# Project 1
# Written by  : Daniel Lynch
# Last Modified : 01/28/2016
#-------------------------------------------------------------------------------

.PHONY: clean all tarball

all: prodCon

clean:
	rm -f consumer producer prodConShared
	rm -f *.o

# These targets take care of the compilation process.
prodCon: 
	gcc -c prodConShared.c
	gcc -o prodConShared prodConShared.c -lpthread -lm
	gcc -c consumer.c
	gcc -o consumer consumer.c -lpthread
	gcc -c producer.c
	gcc -o producer producer.c -lpthread