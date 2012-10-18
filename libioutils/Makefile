all:
	$(MAKE) -C src

test:
	$(MAKE) -C tests
	tests/io_test basic

clean:
	$(MAKE) -C src clean
	$(MAKE) -C tests clean

mrproper:
	$(MAKE) -C src mrproper
	$(MAKE) -C tests mrproper
