.PHONY: all clean run cliente servidor

SOURCE_DIR := source

all: cliente servidor

cliente: $(SOURCE_DIR)/cliente.c $(SOURCE_DIR)/libclaves.so
	gcc $(SOURCE_DIR)/cliente.c $(SOURCE_DIR)/comm.c -L$(SOURCE_DIR) -lclaves -o cliente

servidor: $(SOURCE_DIR)/clavesservidor.c $(SOURCE_DIR)/servidor.c $(SOURCE_DIR)/libclaves.so
	gcc $(SOURCE_DIR)/clavesservidor.c $(SOURCE_DIR)/comm.c $(SOURCE_DIR)/servidor.c -L$(SOURCE_DIR) -lclaves -o servidor

$(SOURCE_DIR)/libclaves.so: $(SOURCE_DIR)/claves.o
	gcc -shared -o $(SOURCE_DIR)/libclaves.so $(SOURCE_DIR)/claves.o

$(SOURCE_DIR)/claves.o: $(SOURCE_DIR)/claves.c
	gcc -c -fPIC $(SOURCE_DIR)/claves.c -o $(SOURCE_DIR)/claves.o

clean:
	rm -f $(SOURCE_DIR)/claves.o $(SOURCE_DIR)/libclaves.so cliente servidor

run-cliente: cliente
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$(SOURCE_DIR) env IP_TUPLAS=localhost PORT_TUPLAS=4500 ./cliente

run-servidor: servidor
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$(SOURCE_DIR) ./servidor 4500

