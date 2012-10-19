LIB=libfautes.so

SRC=$(wildcard *.c)
OBJ=$(SRC:.c=.o)

WARNINGS= -Wall -Wunused -Wunused-value  -Wunused-variable  -Wunused-label \
		-Wno-strict-aliasing -Wextra -Wswitch-default -Wswitch-enum \
		-Wwrite-strings -Wdeclaration-after-statement \
		-Wunsafe-loop-optimizations -Wmissing-prototypes -Wformat-security \
		-Winit-self -Wshadow -Wextra

CFLAGS+= -O0 -g $(WARNINGS)

CPPFLAGS+=-I../include

all:$(LIB)

$(LIB):$(OBJ)
	$(CC) --shared  -o $@ $^ -Wl,-soname=$@

%.o:%.c
	$(CC) -c -o $@ $< $(CFLAGS) $(CPPFLAGS)

clean:
	rm -f $(OBJ)

mrproper:clean
	rm -f $(LIB)
