#include <stdio.h>

typedef struct {
    int seq_no;
    int is_ack;
    int is_syn;
    int is_fin;
    int ack_no;
    char* data;
    //int checksum;
}Packet;

Packet* make_packet(int seq_no,int is_ack,char* data){

    Packet* packet;
    packet->seq_no = seq_no;
    packet->is_ack = is_ack;
    packet->data = data;
    //packet->checksum
    return packet;
}

