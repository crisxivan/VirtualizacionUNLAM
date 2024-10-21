.PHONY: clean

comp=g++

clienteComp: cliente.o
	$(comp) -o clienteComp cliente.o

Cliente.o: cliente.cpp
	$(comp) -c cliente.cpp
	
clean:
	rm *.o