.PHONY: clean

comp=g++

servidorComp: Servidor.o
	$(comp) -o servidorComp Servidor.o

Servidor.o: Servidor.cpp
	$(comp) -c Servidor.cpp


	
clean:
	rm *.o