/** ************************************ 
* This file contains functions for creating sockets
* to listen for clients on port 8053 and connecting
* to the upstream via the IPV4 address and port number
* provided by the command line. 

* These functions also accept type AAAA DNS queries 
* from clients, forward them to the upstream server, 
* accept DNS responses from the upstream server and 
* send the responses back to the clients based on conditions 
* outlined by the specification.

* Student Name: Le Minh Truong
* Student ID: 1078113
* Subject Name: Computer Systems
* Subject Code: COMP30023
* Project 2

**************************************** */

#include "header.h"

/* Create socket for listening for clients, accepting DNS queries
from clients and send back DNS responses to clients. */
void open_socket_client(char *server_port, char* server_ip){
    int sockfd, newsockfd, re, s;
	struct addrinfo hints, *res;
	struct sockaddr_storage client_addr;
	socklen_t client_addr_size;

    int msg_len_num_bytes, msg_len, resp;

    uint8_t bytes_for_len[2];
    uint8_t *buffer;
    uint8_t *server_resp;
    dnsQuery_t dnsQuery;

    uint8_t resp_num_bytes[2];
    dnsResponse_t dnsResponse;

    // Create log file
    FILE *fp;
    fp = fopen("dns_svr.log","w");
    
    /** *********************************/

     /** The following section of code is based
     * on the program "server.c", written by 
     * Steven Tang as an illustration for Week 9 
     * Practical of the subject COMP30023
     * 
     * The link to the program is as followed:
     * https://gitlab.eng.unimelb.edu.au/comp30023-2021-projects/practicals/tree/master/week9
     * */

    /**********************************/
    
    // Create address to listen on with port CLIENT_PORT defined in header.h
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;       // IPv4
	hints.ai_socktype = SOCK_STREAM; // TCP
	hints.ai_flags = AI_PASSIVE;     // for bind, listen, accept

	// Create node
	s = getaddrinfo(NULL, CLIENT_PORT, &hints, &res);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}

	// Create socket
	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sockfd < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	// Reuse port if possible
	re = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &re, sizeof(int)) < 0) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	// Bind address to the socket
	if (bind(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
		perror("bind");
		exit(EXIT_FAILURE);
	}
	freeaddrinfo(res);

	// Listen on socket for connections
	if (listen(sockfd, 5) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

    /* Keep socket open to accept multiple AAAA queries */
    while(TRUE){
        // Accept a connection - blocks until a connection is ready to be accepted
	    // Get back a new file descriptor to communicate on
        client_addr_size = sizeof client_addr;
        newsockfd =
            accept(sockfd, (struct sockaddr*)&client_addr, &client_addr_size);
        if (newsockfd < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        /******************************************************/

        buffer = (uint8_t*)malloc(sizeof(buffer)*BUFFER_SZ);
        server_resp = (uint8_t*)malloc(sizeof(server_resp)*BUFFER_SZ);

        // Read the first 2 bytes of the query to determine the length of
        // the DNS query
        msg_len_num_bytes = read(newsockfd, bytes_for_len, 2);

        // Ensure that the server successfully reads the first 2 bytes
        while(msg_len_num_bytes < 2){
            msg_len_num_bytes += read(newsockfd, bytes_for_len + 1, 2 - msg_len_num_bytes);
        }

        // Convert the first 2 bytes to decimal 
        dnsQuery.packet_len = (uint16_t*)bytes_for_len;
        msg_len = ntohs(*(dnsQuery.packet_len));

        // Parse the DNS query 
        parse_query(&dnsQuery, buffer, msg_len, newsockfd);

        // Log the DNS query
        log_request(fp, &dnsQuery);

        // Error handling for non-AAAA query
        if(ntohs(dnsQuery.dnsQuestion.qtype) != TYPE_AAAA){

            buffer[2] = buffer[2] | 0x80; //Set QR to 1
            buffer[3] = buffer[3] | 0x04; //Set RCODE to 4
            buffer[3] = buffer[3] | 0x80; //Set RA to 1

            // Send back length of DNS response to client
            int err_resp, err_resp_msg;
            err_resp = write(newsockfd, bytes_for_len, 2);

            // Ensure that the length of the response is sent to
            // the client correctly
            while(err_resp < 0){
                err_resp = write(newsockfd, bytes_for_len, 2);
            }
            while(err_resp < 2){
                err_resp += write(newsockfd, bytes_for_len + err_resp, 2 - err_resp);
            }

            // Send back the DNS response with RCODE set to 4 to client
            err_resp_msg = write(newsockfd, buffer, ntohs(*(dnsQuery.packet_len)));

            // Ensure that the DNS server sends the DNS response back to client
            // correctly
            while(err_resp_msg < 0){
                err_resp_msg = write(newsockfd, buffer, ntohs(*(dnsQuery.packet_len)));
            }
            while(err_resp_msg < ntohs(*(dnsQuery.packet_len))){
                err_resp_msg += write(newsockfd, buffer + err_resp_msg,//
                 ntohs(*(dnsQuery.packet_len)) - err_resp_msg);
            }

            free(dnsQuery.dnsQuestion.qname);
            free(buffer);
            free(server_resp);
            continue;
        }

        // Forward DNS query to the upstream server, receive DNS response from
        // the upstream server and parse the DNS response.
        open_socket_server(server_port, server_ip, bytes_for_len, resp_num_bytes,//
         msg_len, &dnsResponse, buffer, server_resp);

        // Log the DNS response
        log_response(fp, &dnsResponse);

        // Send the length of the DNS response back to the client
        resp = write(newsockfd, resp_num_bytes, 2);

        // Ensure that the DNS server sends the length of the DNS response
        // back to the client correctly
        while(resp < 0){
            resp = write(newsockfd, resp_num_bytes, 2);
        }
        while(resp < 2){
            resp += write(newsockfd, resp_num_bytes + resp, 2 - resp);
        }

        // Send the DNS response back to the client
        int new_resp;
        new_resp = write(newsockfd, server_resp, ntohs(*(dnsResponse.packet_len)));

        // Ensure that the DNS server sends the DNS response back to the client 
        // correctly
        while(new_resp < 0){
            new_resp = write(newsockfd, server_resp, ntohs(*(dnsResponse.packet_len)));
        }
        while(new_resp < ntohs(*(dnsResponse.packet_len))){
            new_resp += write(newsockfd, server_resp + new_resp, ntohs(*(dnsResponse.packet_len)) - new_resp);
        }

        free(dnsQuery.dnsQuestion.qname);
        free(dnsResponse.dnsQuestion.qname);
        free(dnsResponse.dnsAnswer.rdata);
        free(buffer);
        free(server_resp);
    }

    fclose(fp);

    // Close socket
	close(sockfd);
	close(newsockfd);
}



void open_socket_server(char *port, char* server_ip, uint8_t *bytes_for_len, uint8_t *resp_num_bytes, //
int msg_len, dnsResponse_t *dnsResponse, uint8_t *buffer, uint8_t* server_resp){
    int sockfd, s;
	struct addrinfo hints, *servinfo, *rp;

    int fwd_bytes, fwd_req;
    int rcv_bytes, resp_len;

    /** *********************************/

     /** The following section of code is based
     * on the program "client.c", written by 
     * Steven Tang as an illustration for Week 9 
     * Practical of the subject COMP30023
     * 
     * The link to the program is as followed:
     * https://gitlab.eng.unimelb.edu.au/comp30023-2021-projects/practicals/tree/master/week9
     * */

    /**********************************/

    memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	// Get addrinfo of server. 
	s = getaddrinfo(server_ip, port, &hints, &servinfo);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}

	// Connect to first valid result
	for (rp = servinfo; rp != NULL; rp = rp->ai_next) {
		sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sockfd == -1)
			continue;

		if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) != -1)
			break; // success

		close(sockfd);
	}
	if (rp == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		exit(EXIT_FAILURE);
	}
	freeaddrinfo(servinfo);

    /**************************************************/

    //Forward DNS query's length to upstream server
    fwd_bytes = write(sockfd, bytes_for_len, 2);

    // Ensure that the DNS server forwards the DNS query's length
    // to upstream server correctly.
    while(fwd_bytes < 0){
        fwd_bytes = write(sockfd, bytes_for_len, 2);
    }
    while(fwd_bytes < 2){
        fwd_bytes += write(sockfd, bytes_for_len + fwd_bytes, 2 - fwd_bytes);
    }

    // Forward DNS query to upstream server
    fwd_req = write(sockfd, buffer, msg_len);

    // Ensure that the DNS server forwards the DNS query
    // to the upstream server correctly
    while(fwd_req < 0){
        fwd_req = write(sockfd, buffer, msg_len);
    }
    while(fwd_req < msg_len){
        fwd_req += write(sockfd, buffer + fwd_req, msg_len - fwd_req);
    }


    // Read the length of DNS response received from upstream server
    rcv_bytes = read(sockfd, resp_num_bytes, 2);

    // Ensure that the DNS server reads the length of DNS response 
    // corretly.
    while(rcv_bytes < 0){
        rcv_bytes = read(sockfd, resp_num_bytes, 2);
    }
	while(rcv_bytes < 2){
		rcv_bytes += read(sockfd, resp_num_bytes + 1, 2 - rcv_bytes);
	}

    // Converts DNS response's length to decimal
    dnsResponse->packet_len = (uint16_t*)resp_num_bytes;
    resp_len = ntohs(*(dnsResponse->packet_len));

    // Parse the DNS response
    parse_response(dnsResponse, server_resp, resp_len, sockfd);

    // Close the server socket
    close(sockfd);
}