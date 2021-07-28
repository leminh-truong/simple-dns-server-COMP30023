#include <unistd.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>

#define HEADER_LEN 12;

typedef struct {
        uint16_t ID;
        uint16_t flags;
        uint16_t QDCOUNT;
        uint16_t ANCOUNT;
        uint16_t NSCOUNT;
        uint16_t ARCOUNT;
}dnsHeader_t;

typedef struct {
    uint8_t *qname;
    uint16_t qtype;
    uint16_t qclass;
    uint8_t byte_label;
}dnsQuestion_t;

typedef struct {
    uint16_t aname;
    uint8_t *atype;
    uint16_t aclass;
    uint32_t ttl;
    uint16_t rdlength;
    uint8_t *rdata;
    uint8_t *ar;
}dnsAnswer_t;

typedef struct{
    uint16_t qr, opcode, aa, tc, rd, ra, z, rcode;
}headerFlags_t;

uint16_t concat_byte(uint8_t byte1, uint8_t byte2);
uint32_t concat_4_byte(uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4);
void parse_header(dnsHeader_t *dnsHeader, uint8_t *buffer);
void parse_question(dnsQuestion_t *dnsQuestion, uint8_t *buffer, uint8_t qlen, int *curr_index);
void parse_answer(dnsAnswer_t *dnsAnswer, uint8_t *buffer, uint16_t rdlength, int* curr_index);


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

    num_byte_label = buffer[12];
    dnsQuestion.byte_label = buffer[13];
    curr_index = 14;
    qlen = buffer[curr_index];

    parse_question(&dnsQuestion, buffer, qlen, &curr_index);
    
    printf("curr in is: %d\n", curr_index);
    printf("curr byte is: %02x\n", buffer[curr_index]);
    printf("last byte is: %02x\n\n", buffer[curr_index-1]);

    printf("Answer name\n");
    dnsAnswer.aname = concat_byte(buffer[curr_index], buffer[curr_index+1]);
    curr_index += 2;
    printf("curr in is: %d\n", curr_index);
    printf("curr byte is: %02x\n", buffer[curr_index]);

    printf("AnswerType\n");
    dnsAnswer.atype = ntohs(concat_byte(buffer[curr_index], buffer[curr_index+1]));
    curr_index += 2;
    printf("curr in is: %d\n", curr_index);
    printf("curr byte is: %02x\n", buffer[curr_index]);

    printf("Answer Class\n");
    dnsAnswer.aclass =concat_byte(buffer[curr_index], buffer[curr_index+1]);
    curr_index += 2;
    printf("curr in is: %d\n", curr_index);
    printf("curr byte is: %02x\n", buffer[curr_index]);

    printf("Answer TTL\n");
    dnsAnswer.ttl = concat_4_byte(buffer[curr_index], buffer[curr_index+1], buffer[curr_index+2], buffer[curr_index+3]);
    curr_index += 4;
    printf("curr in is: %d\n", curr_index);
    printf("ttl is : %04x\n",dnsAnswer.ttl );
    printf("curr byte is: %02x\n", buffer[curr_index]);

    printf("RD Length\n");
    dnsAnswer.rdlength = ntohs(concat_byte(buffer[curr_index], buffer[curr_index+1]));
    curr_index += 2;
    printf("curr in is: %d\n", curr_index);
    printf("%d\n",dnsAnswer.rdlength);

    parse_answer(&dnsAnswer, buffer, dnsAnswer.rdlength, &curr_index);

    printf("bytes remaining: %d\n", msg_bytes - curr_index);
    printf("IPv6: %02hhn\n",dnsAnswer.rdata);
    free(dnsAnswer.rdata);
    free(dnsQuestion.qname);
    free(buffer);
    return 0;
}




void parse_header(dnsHeader_t *dnsHeader, uint8_t *buffer){
    dnsHeader->ID = concat_byte(buffer[0], buffer[1]);
    
    for(int i=2;i<12;i++){
        if(i==2){
            printf("This is flags\n");	
            dnsHeader->flags = concat_byte(buffer[i], buffer[i+1]);
        }
        else if(i==4){
	    printf("This is QDCOUNT\n");
            dnsHeader->QDCOUNT = concat_byte(buffer[i], buffer[i+1]);
        }
        else if(i==6){
	    printf("This is ANCOUNT\n");
            dnsHeader->ANCOUNT = concat_byte(buffer[i], buffer[i+1]);
        }
        else if(i==8){
	    printf("This is NSCOUNT\n");
            dnsHeader->NSCOUNT = concat_byte(buffer[i], buffer[i+1]);
        }
        else if(i==10){
	    printf("This is ARCOUNT\n");
            dnsHeader->ARCOUNT =concat_byte(buffer[i], buffer[i+1]);
        }
    }
}

void parse_question(dnsQuestion_t *dnsQuestion, uint8_t *buffer, uint8_t qlen, int* curr_index){
    dnsQuestion->qname = (uint8_t*)malloc(sizeof(dnsQuestion->qname)*(qlen + 1));
    for(int j=1; j <= qlen; j++){
        dnsQuestion->qname[j-1] = buffer[*curr_index+j];
    }
    *curr_index = *curr_index + qlen;
    dnsQuestion->qname[qlen] = buffer[*curr_index + 1];
    *curr_index += 2;
    dnsQuestion->qtype = concat_byte(buffer[*curr_index], buffer[*curr_index + 1]);
    *curr_index += 2;
    dnsQuestion->qclass = concat_byte(buffer[*curr_index], buffer[*curr_index + 1]);
    *curr_index += 2;
    printf("%s\n",dnsQuestion->qname);
}

void parse_answer(dnsAnswer_t *dnsAnswer, uint8_t *buffer, uint16_t rdlength, int *curr_index){
    dnsAnswer->rdata = (uint8_t*)malloc(sizeof(dnsAnswer->rdata)*(rdlength+1));
    for(int j=1; j <= rdlength; j++){
        dnsAnswer->rdata[j-1] = buffer[*curr_index+j];
    }
    *curr_index += rdlength;
}

uint16_t concat_byte(uint8_t byte1, uint8_t byte2){
    uint16_t concat = byte1 << 8 | byte2;
    printf("concat with ntohs: %d\n", ntohs(concat));
    printf("hex: %02x\n", concat);
    printf("concat no ntohs: %d\n", concat);
    return concat;
}

uint32_t concat_4_byte(uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4){
    uint16_t concat1 = byte1 << 8 | byte2;
    uint16_t concat2 = byte3 << 8 | byte4;
    uint32_t concat = concat1 << 16 | concat2;
    return concat;
}