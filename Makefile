#The Makefile of the program

#Student Name: Le Minh Truong
#Student ID: 1078113
#Subject Name: Computer Systems
#Subject Code: COMP30023

#Project 2


dns_svr: dns_server.o socket_open.o parse_info.o
	gcc -Wall -o dns_svr dns_server.o socket_open.o parse_info.o -lm

dns_server.o: dns_server.c header.h
	gcc -c -Wall dns_server.c -lm

socket_open.o: socket_open.c header.h
	gcc -c -Wall socket_open.c -lm

parse_info.o: parse_info.c header.h
	gcc -c -Wall parse_info.c -lm

clean:
	rm -f *.o 
	rm -f dns_svr
	rm -f dns_svr.log