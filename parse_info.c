/** ************************************ 
* This file contains functions to parse different
* sections of DNS queries/responses, as well as 
* functions to write logs while the DNS server 
* functions.

* Student Name: Le Minh Truong
* Student ID: 1078113
* Subject Name: Computer Systems
* Subject Code: COMP30023
* Project 2

**************************************** */


#include "header.h"

/* Function to parse a DNS query */
void parse_query(dnsQuery_t *dnsQuery, uint8_t *buffer, int msg_len, int sockfd){

    int msg_bytes;
	int curr_index;

    // Read the DNS query
    msg_bytes = read(sockfd, buffer, msg_len);

    // Ensure that the DNS server reads the DNS query correctly.
	while(msg_bytes < 0){
		msg_bytes = read(sockfd, buffer, msg_len);
	}
    while(msg_bytes < msg_len){
        msg_bytes += read(sockfd, buffer + msg_bytes, msg_len - msg_bytes);
    }
	
    // Parse header section of the DNS query
    parse_header(&(dnsQuery->dnsHeader), buffer);

    // Parse question section of the DNS query
    parse_question(&(dnsQuery->dnsQuestion), buffer, msg_len, &curr_index);
}


/* Function to parse a DNS response */
void parse_response(dnsResponse_t *dnsResponse, uint8_t *server_resp, int resp_len, int sockfd){

    int resp_bytes;
	int curr_index;

    // Read the DNS response
    resp_bytes = read(sockfd, server_resp, resp_len);

    // Ensure that the DNS server reads the DNS response correctly
	while(resp_bytes < 0){
		resp_bytes = read(sockfd, server_resp, resp_len);
	}
	while(resp_bytes < resp_len){
        resp_bytes += read(sockfd, server_resp + resp_bytes, resp_len - resp_bytes);
    }

    // Parse header section of DNS response
    parse_header(&(dnsResponse->dnsHeader), server_resp);

    // Parse question section of DNS response
    parse_question(&(dnsResponse->dnsQuestion), server_resp, resp_len, &curr_index);

    // Parse answer section of DNS response
    parse_answer(&(dnsResponse->dnsAnswer), server_resp, curr_index);
}

/* Function to parse the header section of DNS query and response*/
void parse_header(dnsHeader_t *dnsHeader, uint8_t *buffer){

    memcpy(dnsHeader, buffer, HEADER_LEN);
}

/* Function to parse the question section of DNS query and response */
void parse_question(dnsQuestion_t* dnsQuestion, uint8_t *buffer, int msg_len, int* curr_index){
    int qlen;

    dnsQuestion->qname = (uint8_t*)malloc(sizeof(dnsQuestion->qname)*BUFFER_SZ);
    qlen = parse_qname(buffer, dnsQuestion->qname);

    memcpy(&dnsQuestion->qtype, buffer + 12 + qlen + 1, 2);
    memcpy(&dnsQuestion->qclass, buffer + 12 + qlen + 1 + 2, 2);

    *curr_index = 12 + qlen + 1 + 2 + 2;
}

/* Function to parse the answer section of DNS response */
void parse_answer(dnsAnswer_t *dnsAnswer, uint8_t *buffer, int curr_index){
    char *ip;
    memcpy(&dnsAnswer->name, buffer + curr_index, 2);
    memcpy(&dnsAnswer->type, buffer + (curr_index + 2), 2);
    memcpy(&dnsAnswer->class, buffer + (curr_index + 2 + 2), 2);
    memcpy(&dnsAnswer->ttl, buffer + (curr_index + 2 + 2 + 2), 4);
    memcpy(&dnsAnswer->rdlength, buffer + (curr_index +2 + 2 + 2 + 4), 2);

    dnsAnswer->rdata = (uint8_t*)malloc(sizeof(dnsAnswer->rdata)*(ntohs(dnsAnswer->rdlength)));
    memcpy(dnsAnswer->rdata, buffer + (curr_index +2 + 2 + 2 + 4 + 2), ntohs(dnsAnswer->rdlength));

    ip = (char*)malloc(sizeof(ip)*IPV6_SZ);
    inet_ntop(AF_INET6, dnsAnswer->rdata, ip, IPV6_SZ);

    free(ip);
} 

/* Function to log DNS requests */
void log_request(FILE *fp, dnsQuery_t *dnsQuery){

    /** *********************************/

     /** The following section of code is based
     * on the program in the website "Tutorialspoint", written 
     * as an illustration for strftime() function 
     * 
     * The link to the program is as followed:
     * https://www.tutorialspoint.com/c_standard_library/c_function_strftime.htm
     * */

    /**********************************/
    time_t rawtime;
    struct tm *time_info;
    char time_buffer[TIME_SZ];

    time( &rawtime );

    time_info = localtime( &rawtime );
    strftime(time_buffer, TIME_SZ, "%FT%T%z", time_info);
    /*******************************************************/

    // Write log to log file for DNS requests
    fprintf(fp,"%s requested %s\n", time_buffer, dnsQuery->dnsQuestion.qname);
    if(ntohs(dnsQuery->dnsQuestion.qtype) != TYPE_AAAA){
        fprintf(fp,"%s unimplemented request\n", time_buffer);
    }
    fflush(fp);
}


/* Function to log DNS responses */
void log_response(FILE *fp, dnsResponse_t *dnsResponse){
    char *ip;

    /** *********************************/

     /** The following section of code is based
     * on the program in the website "Tutorialspoint", written 
     * as an illustration for strftime() function 
     * 
     * The link to the program is as followed:
     * https://www.tutorialspoint.com/c_standard_library/c_function_strftime.htm
     * */

    /**********************************/
    time_t rawtime;
    struct tm *time_info;
    char time_buffer[TIME_SZ];

    ip = (char*)malloc(sizeof(ip)*IPV6_SZ);
    inet_ntop(AF_INET6, dnsResponse->dnsAnswer.rdata, ip, IPV6_SZ);

    time( &rawtime );

    time_info = localtime( &rawtime );
    strftime(time_buffer, TIME_SZ, "%FT%T%z", time_info);

    /****************************************************/

    if(ntohs(dnsResponse->dnsHeader.ANCOUNT) > 0 && ntohs(dnsResponse->dnsAnswer.type) == TYPE_AAAA){
        fprintf(fp,"%s %s is at %s\n", time_buffer, dnsResponse->dnsQuestion.qname, ip);
    }
    fflush(fp);
    free(ip);
}

/* Function to parse question name */
int parse_qname(uint8_t *buffer, uint8_t *qname){
    int total_len = 0;

    uint8_t zerobyte = 0;

    for(int i=12; buffer[i] != zerobyte; i++){
        total_len++;
    }

    // Skip the first byte of qname section
    int dot = buffer[12];
    int none_dot_place = 0;

    // Insert '.' at appropriate places in the question name
    for(int i=0; i < total_len; i++){
        if(dot == 0){
            dot = buffer[i+13] + 1;
            if(i != total_len-1){
                qname[i] = '.';
            }
        }
        else{
            qname[none_dot_place] = buffer[i+13];
        }
        none_dot_place++;
        dot--;
    }
    qname[none_dot_place] = '\0';


    return total_len;
}