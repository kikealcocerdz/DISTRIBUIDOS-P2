all: cliente servidor

cliente: cliente.c libclaves.so
	gcc cliente.c comm.c -L. -lclaves -o cliente

servidor: clavesservidor.c servidor.c libclaves.so
	gcc clavesservidor.c comm.c servidor.c -L. -lclaves -o servidor

libclaves.so: claves.o
	gcc -shared -o libclaves.so claves.o

claves.o: claves.c
	gcc -c -fPIC claves.c -o claves.o

clean:
	rm -f claves.o libclaves.so cliente servidor
	rm -f /dev/mqueue/*
	
run:
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/source/libclaves.so