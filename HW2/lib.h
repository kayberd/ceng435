#ifndef __LIB_H__
#define __LIB_H__


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include "math.h"
#define WIN_SIZE 10
#define MAX_SEQ_NUM WIN_SIZE*2
#define MAX_DATA_SIZE 8

extern unsigned int SEQ_NUM;

typedef struct Packet{
    int seq_no;
    int ack_no;
    unsigned long int checksum;
    char data[MAX_DATA_SIZE];
}Packet;

typedef struct PacketArrayNode{
    Packet packet;
    int is_acked;
    time_t send_time;
}PacketArrayNode;


/* 
    Below code is taken from https://gist.github.com/1995eaton/06ee2dfe7f83ce0d2e7d
    It reads a stdin for dynamic buffer value
*/
char* packet_to_msg(Packet*);
void print_packet(FILE*,Packet*);
static char* read_stdin (void)
{
  size_t cap = 4096, /* Initial capacity for the char buffer */
         len =    0; /* Current offset of the buffer */
  char *buffer = (char*) malloc(cap * sizeof (char));
  int c;

  /* Read char by char, breaking if we reach EOF or a newline */
  while ((c = fgetc(stdin)) != '\n' && !feof(stdin))
    {
      buffer[len] = c;

      /* When cap == len, we need to resize the buffer
       * so that we don't overwrite any bytes
       */
      if (++len == cap)
        /* Make the output buffer twice its current size */
        buffer = (char*) realloc(buffer, (cap *= 2) * sizeof (char));
    }

  /* Trim off any unused bytes from the buffer */
  //buffer = (char*) realloc(buffer, (len + 1) * sizeof (char));

  /* Pad the last byte so we don't overread the buffer in the future */
  buffer[len] = '\n';
  buffer[len+1] = '\0';

  return buffer;
}

int msg_to_packet(char* msg,PacketArrayNode* packets){
    int packet_count = (int) ceil(strlen(msg)/8.0);
    

    
    for(int i=0;i<packet_count;i++){
        packets[SEQ_NUM].packet.seq_no=SEQ_NUM;
        packets[SEQ_NUM].packet.ack_no=-1;
        packets[SEQ_NUM].packet.checksum = SEQ_NUM-1;
        packets[SEQ_NUM].is_acked=0;

        struct timeval tp;
        gettimeofday(&tp, NULL);
        long int ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
        packets[SEQ_NUM].send_time = ms;

        for(int j=0;j<MAX_DATA_SIZE;j++){
            packets[SEQ_NUM].packet.data[j] = msg[i*8+j];
            packets[SEQ_NUM].packet.checksum += msg[i*8+j];
        }
        SEQ_NUM++;
    }
    
    return packet_count;
}

char* packet_to_msg(Packet* packet){
    char* msg = (char*) malloc(sizeof(char)*MAX_DATA_SIZE);
    for(int i=0;i<MAX_DATA_SIZE;i++){
        msg[i] = packet->data[i];
    }
    return msg;
}

void print_packet(FILE* fp,Packet* packet){
    fprintf(fp,"(seq:%d,ack:%d,%c%c%c%c%c%c%c%c)\n",packet->seq_no,packet->ack_no,packet->data[0],packet->data[1],packet->data[2],packet->data[3],packet->data[4],packet->data[5],packet->data[6],packet->data[7]);
}
void print_msg(FILE* stream,char* msg){
    if(strlen(msg))
        for(int i=0;i<MAX_DATA_SIZE;i++)
            fprintf(stream,"%c",msg[i]);
}
unsigned int check_packet_checksum(Packet* packet,long unsigned int *checksum){
    int checked_sum = 0;
    checked_sum += packet->seq_no+packet->ack_no;
    for(int i=0;i<MAX_DATA_SIZE;i++)
        checked_sum+= packet->data[i];
    (*checksum)=checked_sum;
    return checked_sum==packet->checksum;

}
Packet* make_ack(int ack_no){
    Packet* ack = (Packet*) malloc(sizeof(Packet));
    ack->ack_no=ack_no;
    ack->seq_no=-1;
    for(int i=0;i<MAX_DATA_SIZE;i++)
        ack->data[i] = 0;
    ack->checksum=ack_no-1;
    return ack;  
}
int assign_packet(Packet* lhs,Packet* rhs){
    if(lhs && rhs){
        lhs->ack_no=rhs->ack_no;
        lhs->seq_no=rhs->seq_no;
        lhs->checksum=rhs->checksum;
        for(int i=0;i<MAX_DATA_SIZE;i++)
            lhs->data[i] = rhs->data[i];      
        
        return 1;
    }
    return -1;
}
void dump_window(PacketArrayNode* window){

    for(int i=0;i<SEQ_NUM;i++){
        print_packet(stdout,&(window[i].packet));
    }
}











#endif //__LIB.H___