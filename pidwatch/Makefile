pidwatch_tests:libpidwatch.so tests/pidwatch_tests.c include/pidwatch.h
	gcc tests/pidwatch_tests.c -g -O0 -o pidwatch_tests -L. -I include/ -lpidwatch -Wall -Wl,-rpath /home/nicolas/workspace/C/pidwatch

libpidwatch.so:pidwatch.o
	gcc -shared -Wl,-soname,libpidwatch.so -o libpidwatch.so  pidwatch.o
	
pidwatch.o:src/pidwatch.c include/pidwatch.h
	gcc -Wall -g -O0 -c src/pidwatch.c -o pidwatch.o -fPIC -I include/ -Wall -Wextra

clean:
	rm -f libpidwatch.so pidwatch.o pidwatch_tests
