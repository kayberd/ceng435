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
    Packet packet;
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
  buffer[len] = '\0';

  return buffer;
}


int msg_to_packet(char* msg,PacketArrayNode* packets){
    int packet_count = (int) ceil(strlen(msg)/8.0);
    
    //packet_count = 31;
    FILE* fp = fopen("error2.txt","w");
    /*fprintf(fp,"%d",packet_count);
    fclose(fp);
    while(1);    */
    
    for(int i=0;i<packet_count;i++){
        (packets)[i].packet.seq_no=i;
        (packets)[i].packet.ack_no=-1;
        (packets)[i].is_acked=0;
        for(int j=0;j<MAX_DATA_SIZE;j++){
            (packets)[i].packet.data[j] = msg[i*8+j];
        }
        print_packet(fp,&(packets[i].packet));
  
        //fprintf(fp,"%s",packets[i].packet.data);   
    }

    fclose(fp);
    //while(1);
    return packet_count;
}

void packet_to_msg(Packet* packet,char* msg){
    msg = (char*) malloc(sizeof(char)*MAX_DATA_SIZE);
    for(int i=0;i<MAX_DATA_SIZE;i++){
        msg[i] = packet->data[i];
    }
}

void print_packet(FILE* fp,Packet* packet){
    fprintf(fp,"(seq:%d,ack:%d,%c%c%c%c%c%c%c%c)\n",packet->seq_no,packet->ack_no,packet->data[0],packet->data[1],packet->data[2],packet->data[3],packet->data[4],packet->data[5],packet->data[6],packet->data[7]);
}
#endif //__LIB.H___