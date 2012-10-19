VGR=valgrind --tool=memcheck --leak-check=full --show-reachable=yes --track-fds=yes

all:
	$(MAKE) -C src

test:
	$(MAKE) -C tests
	tests/io_test basic

vtest:
	$(MAKE) -C tests
	$(VGR) tests/io_test basic

clean:
	$(MAKE) -C src clean
	$(MAKE) -C tests clean

mrproper:
	$(MAKE) -C src mrproper
	$(MAKE) -C tests mrproper
