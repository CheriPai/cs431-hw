prodConsumSemaphore: prodConsumSemaphore.o
	gcc $(CFLAGS) -o prodConsumSemaphore prodConsumSemaphore.o -lpthread


prodConsumSemaphore.o: prodConsumSemaphore.c
	gcc $(CLFAGS) -c prodConsumSemaphore.c


run:
	./prodConsumSemaphore


clean:
	rm -f *.o prodConsumSemaphore
