/** ************************************ 
* This file is the header file of the program, which contains
* all the necessary declarations for the program to function.

* Student Name: Le Minh Truong
* Student ID: 1078113
* Subject Name: Computer Systems
* Subject Code: COMP30023
* Project 2

**************************************** */

#ifndef HEADER

#define _POSIX_C_SOURCE 200112L
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <assert.h>
#include <time.h>

#define HEADER_LEN 12       // Size of header of a DNS packet
#define CLIENT_PORT "8053"  // Port which clients use to connect to the server
#define BUFFER_SZ 1024      // Size of buffer containing bytes from packets
#define TIME_SZ 80          // Size of buffer containing time information
#define TRUE 1              // Boolean value of TRUE
#define IPV6_SZ 46          // Size of IPV6
#define TYPE_AAAA 28        // Type ID of query/response of type AA

/*****************************************/

/** The following structures are structures of the header,
 * question and answer sections of DNS packets. These structures
 * are partially based on structures defined by Pereira, W.F. 
 * in his Github repository, "simple-dns".  
 * 
 * The link to the repository is as followed:
 * https://github.com/wfelipe/simple-dns
 * */

/*****************************************/
typedef struct {
        uint16_t ID;
        uint16_t rd:1;
		uint16_t tc:1;
		uint16_t aa:1;
		uint16_t opcode:4;
		uint16_t qr:1;
		uint16_t rcode:4;
		uint16_t z:3;
		uint16_t ra:1;
        uint16_t QDCOUNT;
        uint16_t ANCOUNT;
        uint16_t NSCOUNT;
        uint16_t ARCOUNT;
}dnsHeader_t;

typedef struct {
    uint8_t num_byte_label;
    uint8_t *byte_label;
    uint8_t qlen; 
    uint8_t *qname;
    uint16_t qtype;
    uint16_t qclass;
    uint8_t *ar;
}dnsQuestion_t;

typedef struct {
    uint16_t name;
    uint16_t type;
    uint16_t class;
    uint32_t ttl;
    uint16_t rdlength;
    uint8_t *rdata;
}dnsAnswer_t;

/*******************************************************/

/* Structure for query packets */
typedef struct {
    uint16_t *packet_len;
    dnsHeader_t dnsHeader;
    dnsQuestion_t dnsQuestion;
}dnsQuery_t;

/* Structure for answer packets */
typedef struct {
    uint16_t *packet_len;
    dnsHeader_t dnsHeader;
    dnsQuestion_t dnsQuestion;
    dnsAnswer_t dnsAnswer;
}dnsResponse_t;

/* Function prototypes for parsing sections of DNS packets and writing logs. */
void parse_query(dnsQuery_t *dnsQuery, uint8_t *buffer, int msg_len, int sockfd);
void parse_response(dnsResponse_t *dnsResponse, uint8_t *server_resp, int msg_len, int sockfd);
void parse_header(dnsHeader_t *dnsHeader, uint8_t *buffer);
void parse_question(dnsQuestion_t *dnsQuestion, uint8_t *buffer, int msg_len, int* curr_index);
void parse_answer(dnsAnswer_t *dnsAnswer, uint8_t *buffer, int curr_index);
void log_request(FILE *fp, dnsQuery_t *dnsQuery);
void log_response(FILE *fp, dnsResponse_t *dnsResponse);
int get_qname_len(uint8_t* buffer);
int parse_qname(uint8_t* buffer, uint8_t* qname);

/* Function prototype for creating and opening socket for client to connect to the server. */
void open_socket_client(char *server_port, char* server_ip);

/* Function prototype for creating socket to connect to the upstream server. */
void open_socket_server(char *port, char* ip, uint8_t *bytes_for_len, uint8_t *resp_num_bytes, //
int msg_len, dnsResponse_t *dnsResponse, uint8_t *buffer, uint8_t *server_resp);

#endif