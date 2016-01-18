prodConsumSingleThread: prodConsumSingleThread.o
	gcc $(CFLAGS) -o prodConsumSingleThread prodConsumSingleThread.o


prodConsumSingleThread.o: prodConsumSingleThread.c
	gcc $(CLFAGS) -c prodConsumSingleThread.c


run:
	./prodConsumSingleThread


clean:
	rm -f *.o prodConsumSingleThread
