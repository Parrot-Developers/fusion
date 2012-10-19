# verbosity
ifndef V
	Q:=@
endif

CFLAGS   += -g -O0 -Wall -Wextra
CPPFLAGS += -I../include/
LDFLAGS  += -lcunit -ldl

BINARY = fautes

SOURCES = $(wildcard *.c)

all: $(BINARY)

$(BINARY): $(SOURCES)
	$(Q) $(CC) $^ $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@

clean:
	@echo ioutils: cleaning $(TEST_BINARY)
	$(Q) rm -f $(BINARY)
	
mrproper:clean
