/** ************************************ 
* This file contains the main function of the DNS server, 
* which passes the IP address and port number of the
* upstream server for the DNS server to connect to and
* initializes socket creation for listening for clients.

* Student Name: Le Minh Truong
* Student ID: 1078113
* Subject Name: Computer Systems
* Subject Code: COMP30023
* Project 2

**************************************** */

#include "header.h"

int main(int argc, char** argv) {
	char *server_port;
	char *server_ip;

	server_port = (char*)malloc(sizeof(server_port)*BUFFER_SZ);
	server_ip = (char*)malloc(sizeof(server_ip)*BUFFER_SZ);

	strcpy(server_port, argv[2]);
	strcpy(server_ip, argv[1]);

    //Initialize DNS server
    open_socket_client(server_port, server_ip);

    free(server_port);
    free(server_ip);

    return 0;
}
