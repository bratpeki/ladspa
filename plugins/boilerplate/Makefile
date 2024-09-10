build:
	gcc -fPIC -c amp.c -o amp.o
	gcc -shared -o amp.so amp.o -lm -ldl

clean:
	rm *.o *.so
