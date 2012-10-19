VGR=valgrind --tool=memcheck --leak-check=full --show-reachable=yes --track-fds=yes

all:fautes lib

fautes:
	$(MAKE) -C src

lib:
	$(MAKE) -C lib

clean:
	$(MAKE) -C src clean
	$(MAKE) -C lib clean

mrproper:
	$(MAKE) -C src mrproper
	$(MAKE) -C lib mrproper

.PHONY:lib