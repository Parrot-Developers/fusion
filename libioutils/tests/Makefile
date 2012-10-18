# verbosity
ifndef V
	Q:=@
endif

CFLAGS   += -g -O0 -Wall -Wextra
CPPFLAGS += -I../include/
LDFLAGS  += -lcunit -lrs

# Put the path to librs headers and binary lib
CPPFLAGS += -I../../librs/include
LDFLAGS  += -L../../librs/src

TEST_BINARY = io_test

TEST_SOURCES = $(wildcard *.c)

IO_MODULES = $(wildcard ../src/*.c)

IO_DEPENDENCIES =

all: $(TEST_BINARY)

$(TEST_BINARY): $(TEST_SOURCES) $(IO_MODULES) $(IO_DEPENDENCIES)
	$(Q) $(CC) $^ $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@

clean:
	@echo ioutils: cleaning $(TEST_BINARY)
	$(Q) rm -f $(TEST_BINARY)
	
mrproper:clean
