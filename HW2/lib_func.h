#ifndef __LIB_H__
#define __LIB_H__


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "math.h"
#define WIN_SIZE 10
#define MAX_SEQ_NUM WIN_SIZE*2
#define MAX_DATA_SIZE 8

typedef struct Packet{
    int seq_no;
    int ack_no;
    char data[MAX_DATA_SIZE];
    //int checksum;
}Packet;

typedef struct PacketArrayNode{
    Packet* packet;
    int is_acked;
}PacketArrayNode;
/*
Packet* make_packet(int seq_no,,char* data){

    Packet* packet;
    packet->seq_no = seq_no;
    strcpy(packet->data,data);
    //packet->checksum
    return packet;
}
*/


/* 
    Below code is taken from https://gist.github.com/1995eaton/06ee2dfe7f83ce0d2e7d
    It reads a stdin for dynamic buffer value
*/
static char* read_stdin (void)
{
  size_t cap = 4096, /* Initial capacity for the char buffer */
         len =    0; /* Current offset of the buffer */
  char *buffer = malloc(cap * sizeof (char));
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
        buffer = realloc(buffer, (cap *= 2) * sizeof (char));
    }

  /* Trim off any unused bytes from the buffer */
  buffer = realloc(buffer, (len + 1) * sizeof (char));

  /* Pad the last byte so we don't overread the buffer in the future */
  buffer[len] = '\0';

  return buffer;
}


int msg_to_packet(char* msg,PacketArrayNode* packets){
    int packet_count = (int) ceil(strlen(msg)/(float)MAX_DATA_SIZE));
    printf("%d",packet_count);

    packets = (PacketArrayNode*) malloc(sizeof(PacketArrayNode)*packet_count);
    for(int i=0;i<packet_count;i++){
        packets[i].packet->seq_no=i;
        packets[i].packet->ack_no=-1;
        packets[i].is_acked=0;
        for(int j=0;j<MAX_DATA_SIZE;j++)
            packets[i].packet->data[j] = msg[i*MAX_DATA_SIZE+j];     
    }

    return packet_count;
}

void packet_to_msg(Packet* packet,char* msg){
    msg = (char*) malloc(sizeof(char)*MAX_DATA_SIZE);
    for(int i=0;i<MAX_DATA_SIZE;i++){
        msg[i] = packet->data[i];
    }
}

void print_packet(Packet* packet){
    printf("(seq:%d,ack:%d,%s\n)",packet->seq_no,packet->ack_no,packet->data);
}
#endif //__LIB.H___