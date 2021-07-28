#include <unistd.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>

#define HEADER_LEN 12
#define QR_QUERY 0
#define QR_RESPONSE 1
#define OPCODE_QUERY 0 /* a standard query */
#define OPCODE_IQUERY 1 /* an inverse query */
#define OPCODE_STATUS 2 /* a server status request */

// typedef struct {
//         uint16_t ID;
//         uint16_t flags;
//         uint16_t QDCOUNT;
//         uint16_t ANCOUNT;
//         uint16_t NSCOUNT;
//         uint16_t ARCOUNT;
// }dnsHeader_t;

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
    //uint8_t byte_label;
}dnsQuestion_t;

// typedef struct {
//     uint16_t aname;
//     uint8_t *atype;
//     uint16_t aclass;
//     uint32_t ttl;
//     uint16_t rdlength;
//     uint8_t *rdata;
//     uint8_t *ar;
// }dnsAnswer_t;

typedef struct {
    uint16_t name;
    uint16_t type;
    uint16_t class;
    uint32_t ttl;
    uint16_t rdlength;
    uint8_t *rdata;
}dnsAnswer_t;

// typedef struct{
//     uint16_t qr, opcode, aa, tc, rd, ra, z, rcode;
// }headerFlags_t;

// uint16_t concat_byte(uint8_t byte1, uint8_t byte2);
// uint32_t concat_4_byte(uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4);
void parse_header(dnsHeader_t *dnsHeader, uint8_t *buffer);
void parse_question(dnsQuestion_t *dnsQuestion, uint8_t *buffer, int msg_len, int* curr_index);
void parse_answer(dnsAnswer_t *dnsAnswer, uint8_t *buffer, int curr_index);

int main(int argc, char* argv[]){

    uint8_t bytes_for_len[2];
    uint8_t *buffer;
    uint8_t num_byte_label, qlen;
    dnsHeader_t dnsHeader;
    dnsQuestion_t dnsQuestion;
    dnsAnswer_t dnsAnswer;
    int msg_len_num_bytes, msg_len, msg_bytes, curr_index;

    msg_len_num_bytes = read(STDIN_FILENO, bytes_for_len, 2);
    uint16_t *msg_len_seq = (uint16_t*)bytes_for_len;
    msg_len = ntohs(*msg_len_seq);

    buffer = (uint8_t*)malloc(sizeof(buffer)*msg_len);
    msg_bytes = read(STDIN_FILENO, buffer, msg_len);
    printf("bytes read: %d\n",msg_bytes);
   
    parse_header(&dnsHeader, buffer);
    parse_question(&dnsQuestion, buffer, msg_len, &curr_index);
    //parse_answer(&dnsAnswer, buffer, curr_index);
    free(buffer);
    free(dnsQuestion.qname);
    //free(dnsQuestion.ar);
    return 0;

//     num_byte_label = buffer[12];
//     dnsQuestion.byte_label = buffer[12 + num_byte_label];
//     curr_index = 12 + num_byte_label + 1;
//     qlen = buffer[curr_index];

//     parse_question(&dnsQuestion, buffer, qlen, &curr_index);
    
//     // printf("curr in is: %d\n", curr_index);
//     // printf("curr byte is: %02x\n", buffer[curr_index]);
//     // printf("last byte is: %02x\n\n", buffer[curr_index-1]);

//     // printf("Answer name\n");
//     // dnsAnswer.aname = ntohs(concat_byte(buffer[curr_index], buffer[curr_index+1]));
//     // curr_index += 2;
//     // printf("curr in is: %d\n", curr_index);
//     // printf("curr byte is: %02x\n", buffer[curr_index]);

//     // printf("AnswerType\n");
//     // dnsAnswer.atype = ntohs(concat_byte(buffer[curr_index], buffer[curr_index+1]));
//     // curr_index += 2;
//     // printf("curr in is: %d\n", curr_index);
//     // printf("curr byte is: %02x\n", buffer[curr_index]);

//     // printf("Answer Class\n");
//     // dnsAnswer.aclass = ntohs(concat_byte(buffer[curr_index], buffer[curr_index+1]));
//     // curr_index += 2;
//     // printf("curr in is: %d\n", curr_index);
//     // printf("curr byte is: %02x\n", buffer[curr_index]);

//     // printf("Answer TTL\n");
//     // dnsAnswer.ttl = ntohs(concat_4_byte(buffer[curr_index], buffer[curr_index+1], buffer[curr_index+2], buffer[curr_index+3]));
//     // curr_index += 4;
//     // printf("curr in is: %d\n", curr_index);
//     // printf("ttl is : %04x\n",dnsAnswer.ttl );
//     // printf("curr byte is: %02x\n", buffer[curr_index]);

//     // printf("RD Length\n");
//     // dnsAnswer.rdlength = ntohs(concat_byte(buffer[curr_index], buffer[curr_index+1]));
//     // curr_index += 2;
//     // printf("curr in is: %d\n", curr_index);
//     // printf("%d\n",dnsAnswer.rdlength);

//     // parse_answer(&dnsAnswer, buffer, dnsAnswer.rdlength, &curr_index);

//     // printf("bytes remaining: %d\n", msg_bytes - curr_index);
//     // printf("IPv6: %02hhn\n",dnsAnswer.rdata);
//     // free(dnsAnswer.rdata);
//     free(dnsQuestion.qname);
//     free(buffer);
//     return 0;
}


void parse_header(dnsHeader_t *dnsHeader, uint8_t *buffer){
    buffer[3] = buffer[3] | 0x04;
    memcpy(dnsHeader, buffer, 12);
    printf("id is: %d\n", ntohs(dnsHeader->ID));

    printf("qr is: %d\n", dnsHeader->qr);
    printf("opcode is: %d\n", dnsHeader->opcode);
    printf("aa is: %d\n", dnsHeader->aa);
    printf("tc is: %d\n", dnsHeader->tc);
    printf("rd is: %d\n", dnsHeader->rd);
    printf("ra is: %d\n", dnsHeader->ra);
    printf("z is: %d\n", dnsHeader->z);
    printf("rcode is: %d\n", dnsHeader->rcode);

    printf("qdcount is: %d\n", ntohs(dnsHeader->QDCOUNT));
    printf("ancount is: %d\n", ntohs(dnsHeader->ANCOUNT));
    printf("nscount is: %d\n",ntohs(dnsHeader->NSCOUNT));
    printf("arcount is: %d\n", ntohs(dnsHeader->ARCOUNT));
}

void parse_question(dnsQuestion_t* dnsQuestion, uint8_t *buffer, int msg_len, int* curr_index){
    memcpy(&dnsQuestion->num_byte_label, buffer + 12, 1);
    dnsQuestion->byte_label = (uint8_t*)malloc(sizeof(dnsQuestion->byte_label)*(dnsQuestion->num_byte_label));

    memcpy(dnsQuestion->byte_label, buffer + 13, dnsQuestion->num_byte_label);
    dnsQuestion->byte_label[dnsQuestion->num_byte_label] = '\0';

    memcpy(&dnsQuestion->qlen, buffer + (13 + dnsQuestion->num_byte_label), 1);

    dnsQuestion->qname = (uint8_t*)malloc(sizeof(dnsQuestion->qname)*(dnsQuestion->qlen+1));
    memcpy(dnsQuestion->qname, buffer + (13 + dnsQuestion->num_byte_label + 1), dnsQuestion->qlen + 1);

    memcpy(&dnsQuestion->qtype, buffer + (13 + dnsQuestion->num_byte_label + 1 + dnsQuestion->qlen + 1), 2);
    memcpy(&dnsQuestion->qclass, buffer + (13 + dnsQuestion->num_byte_label + 1 + dnsQuestion->qlen + 1 + 2), 2);

    *curr_index = 13 + dnsQuestion->num_byte_label + 1 + dnsQuestion->qlen + 1 + 2 + 2;
    //printf("buffer + 13: %02x\n",*(buffer + 12));

    int msg_left = msg_len - (15 + dnsQuestion->qlen + 1 + 2 + 2);
    // printf("message left: %d\n", msg_left);
    // printf("last byte: %02x\n", buffer[51]);
    // dnsQuestion->ar = (uint8_t*)malloc(sizeof(dnsQuestion->ar)*msg_left);
    // memcpy(dnsQuestion->ar, buffer + (15 + dnsQuestion->qlen + 1 + 2 + 2), msg_left);

    *curr_index = 15 + dnsQuestion->qlen + 1 + 2 + 2;

    // memcpy(dnsQuestion, buffer + 12, msg_len - 12);
    printf("num byte label: %d\n", dnsQuestion->num_byte_label);
    printf("byte label %s\n",dnsQuestion->byte_label);
    printf("qlen: %d\n", dnsQuestion->qlen);
    printf("qname: %s.%s\n", dnsQuestion->byte_label, dnsQuestion->qname);
    printf("qclass: %02x\n", ntohs(dnsQuestion->qclass));
    printf("qtype: %d\n", ntohs(dnsQuestion->qtype));
    //printf("ar %02x\n", dnsQuestion->ar[msg_left -1]);

    if(ntohs(dnsQuestion->qtype) != 28){
        printf("UNIMPLEMENTED REQUEST\n");
    }
    else{
        printf("REQUEST APPROVED\n");
    }

}

void parse_answer(dnsAnswer_t *dnsAnswer, uint8_t *buffer, int curr_index){
    char *ip;
    memcpy(&dnsAnswer->name, buffer + curr_index, 2);
    memcpy(&dnsAnswer->type, buffer + (curr_index + 2), 2);
    memcpy(&dnsAnswer->class, buffer + (curr_index + 2 + 2), 2);
    memcpy(&dnsAnswer->ttl, buffer + (curr_index + 2 + 2 + 2), 4);
    memcpy(&dnsAnswer->rdlength, buffer + (curr_index +2 + 2 + 2 + 4), 2);
    dnsAnswer->rdata = (uint8_t*)malloc(sizeof(dnsAnswer->rdata)*(ntohs(dnsAnswer->rdlength)));
    memcpy(dnsAnswer->rdata, buffer + (curr_index +2 + 2 + 2 + 4 + 2), ntohs(dnsAnswer->rdlength));

    ip = (char*)malloc(sizeof(ip)*46);
    inet_ntop(AF_INET6, dnsAnswer->rdata, ip, 46);

    printf("compressed name: %04x\n", ntohs(dnsAnswer->name));
    printf("answer type: %04x\n", ntohs(dnsAnswer->type));
    printf("answer class: %04x\n", ntohs(dnsAnswer->class));
    printf("answer ttl: %04x\n", dnsAnswer->ttl);
    printf("rd length: %04x\n", ntohs(dnsAnswer->rdlength));
    printf("rdata %s\n", ip);
} 

// void parse_header(dnsHeader_t *dnsHeader, uint8_t *buffer){
//     dnsHeader->ID = concat_byte(buffer[0], buffer[1]);
    
//     for(int i=2;i<12;i++){
//         if(i==2){
//             printf("This is flags\n");	
//             dnsHeader->flags = concat_byte(buffer[i], buffer[i+1]);
//         }
//         else if(i==4){
// 	    printf("This is QDCOUNT\n");
//             dnsHeader->QDCOUNT = concat_byte(buffer[i], buffer[i+1]);
//         }
//         else if(i==6){
// 	    printf("This is ANCOUNT\n");
//             dnsHeader->ANCOUNT = concat_byte(buffer[i], buffer[i+1]);
//         }
//         else if(i==8){
// 	    printf("This is NSCOUNT\n");
//             dnsHeader->NSCOUNT = concat_byte(buffer[i], buffer[i+1]);
//         }
//         else if(i==10){
// 	    printf("This is ARCOUNT\n");
//             dnsHeader->ARCOUNT =concat_byte(buffer[i], buffer[i+1]);
//         }
//     }
// }

// void parse_question(dnsQuestion_t *dnsQuestion, uint8_t *buffer, uint8_t qlen, int* curr_index){
//     dnsQuestion->qname = (uint8_t*)malloc(sizeof(dnsQuestion->qname)*(qlen + 1));
//     for(int j=1; j <= qlen; j++){
//         dnsQuestion->qname[j-1] = buffer[*curr_index+j];
//     }
//     *curr_index = *curr_index + qlen;
//     dnsQuestion->qname[qlen] = buffer[*curr_index + 1];
//     *curr_index += 2;
//     dnsQuestion->qtype = concat_byte(buffer[*curr_index], buffer[*curr_index + 1]);
//     *curr_index += 2;
//     dnsQuestion->qclass = concat_byte(buffer[*curr_index], buffer[*curr_index + 1]);
//     *curr_index += 2;
//     printf("%s\n",dnsQuestion->qname);
// }

// void parse_answer(dnsAnswer_t *dnsAnswer, uint8_t *buffer, uint16_t rdlength, int *curr_index){
//     dnsAnswer->rdata = (uint8_t*)malloc(sizeof(dnsAnswer->rdata)*(rdlength+1));
//     for(int j=1; j <= rdlength; j++){
//         dnsAnswer->rdata[j-1] = buffer[*curr_index+j];
//     }
//     *curr_index += rdlength;
// }

// uint16_t concat_byte(uint8_t byte1, uint8_t byte2){
//     uint16_t concat = byte1 << 8 | byte2;
//     printf("concat with ntohs: %d\n", ntohs(concat));
//     printf("hex with ntohs: %04x\n", ntohs(concat));
//     printf("hex: %04x\n", concat);
//     printf("concat no ntohs: %d\n\n", concat);
//     return concat;
// }

// uint32_t concat_4_byte(uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4){
//     uint16_t concat1 = byte1 << 8 | byte2;
//     uint16_t concat2 = byte3 << 8 | byte4;
//     uint32_t concat = concat1 << 16 | concat2;
//     return concat;
// }