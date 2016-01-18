prodConsumMutex: prodConsumMutex.o
	gcc $(CFLAGS) -o prodConsumMutex prodConsumMutex.o -lpthread


prodConsumMutex.o: prodConsumMutex.c
	gcc $(CLFAGS) -c prodConsumMutex.c


run:
	./prodConsumMutex


clean:
	rm -f *.o prodConsumMutex
