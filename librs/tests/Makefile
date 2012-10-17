# verbosity
ifndef V
	Q:=@
endif

CFLAGS   += -g -O0 -Wall -Wextra
CPPFLAGS += -DCUNIT_TESTS -I../include/
LDFLAGS  += -lcunit

TEST_BINARY = rs_test

TEST_SOURCES = $(wildcard *.c)

RS_MODULES = $(wildcard ../src/*.c)

RS_DEPENDENCIES =

all: $(TEST_BINARY)

$(TEST_BINARY): $(TEST_SOURCES) $(RS_MODULES) $(RS_DEPENDENCIES)
	$(Q) $(CC) $^ $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@

clean:
	@echo librs: cleaning $(TEST_BINARY)

mrproper:
	@echo librs: deep cleaning $(TEST_BINARY)
	rm -f $(TEST_BINARY)