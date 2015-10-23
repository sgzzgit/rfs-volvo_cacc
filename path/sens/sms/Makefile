all: 
	make -C src
	make -C examples
clean:
	make -C src clean
	make -C examples clean
install:
	mkdir -p test
	make -C src install
	make -C examples install

