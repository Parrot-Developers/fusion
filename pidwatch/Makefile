main:libpidwatch.so example/main.c include/pidwatch.h
	gcc example/main.c -g -O0 -o main -L. -I include/ -lpidwatch -Wall -Wl,-rpath /home/nicolas/workspace/C/pidwatch

libpidwatch.so:pidwatch.o
	gcc -shared -Wl,-soname,libpidwatch.so -o libpidwatch.so  pidwatch.o
	
pidwatch.o:src/pidwatch.c include/pidwatch.h
	gcc -Wall -g -O0 -c src/pidwatch.c -o pidwatch.o -fPIC -I include/ -Wall -Wextra

clean:
	rm libpidwatch.so pidwatch.o main
