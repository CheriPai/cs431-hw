a: a.o
	gcc $(CFLAGS) -o a a.o -lpthread


a.o: a.c
	gcc $(CLFAGS) -c -g a.c


b: b.o
	gcc $(CFLAGS) -o b b.o -lpthread


b.o: b.c
	gcc $(CLFAGS) -c -g b.c

clean:
	rm -f *.o a b
