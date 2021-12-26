#ifndef __LIB_H__
#define __LIB_H__

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <sys/time.h>
#include "math.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

using namespace std;
#define MAX_SEQ_NUM WIN_SIZE*2
#define MAX_DATA_SIZE 8



//#define SERV_PORT 1881
//#define CLI_PORT  1938
#define WIN_SIZE 4
#define MAX_PACKET_NUM 1024
#define TIMEOUT 100 //timeout currently 1000ms 1s

// Driver code

extern int SEQ_NUM;

typedef struct Packet{
    int seq_no;
    int ack_no;
    int checksum;
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
string& packet_to_msg(Packet*);
void print_packet(FILE*,Packet*);


int msg_to_packet(string msg,PacketArrayNode* packets){
    int packet_count = (int) ceil(msg.length()/8.0);
    

    //cout << msg.length() << endl;
    for(int i=0;i<packet_count;i++){
        //printf("SEQ_NUM:%d\n",SEQ_NUM);
        packets[SEQ_NUM].packet.seq_no=SEQ_NUM;
        packets[SEQ_NUM].packet.ack_no=-1;
        packets[SEQ_NUM].packet.checksum = SEQ_NUM-1;
        packets[SEQ_NUM].is_acked=0;

       

        for(int j=0;j<MAX_DATA_SIZE;j++){
            if(j >= msg.length()){
                packets[SEQ_NUM].packet.data[j]=0;
                
            }
            else{
                packets[SEQ_NUM].packet.data[j] = msg[i*8+j];
                packets[SEQ_NUM].packet.checksum += msg[i*8+j];
            }

        }
        SEQ_NUM++;
    }
    
    return packet_count;
}

string& packet_to_msg(Packet* packet){
    string msg;
    for(int i=0;i<MAX_DATA_SIZE;i++){
        msg[i] = packet->data[i];
    }
    return msg;
}

void print_packet(FILE* fp,Packet* packet){
    if(packet->ack_no == -1)
        fprintf(fp,"(seq:%d,%c%c%c%c%c%c%c%c)\n",packet->seq_no,packet->data[0],packet->data[1],packet->data[2],packet->data[3],packet->data[4],packet->data[5],packet->data[6],packet->data[7]);
    else
        fprintf(fp,"ACKED%d\n",packet->ack_no);
}
void print_msg(FILE* stream,string msg){
    if(msg.length() > 0)
        for(int i=0;i<MAX_DATA_SIZE;i++)
            fprintf(stream,"%c",msg[i]);
    //stream << msg;
}
bool check_packet_checksum(Packet packet){
    int checked_sum = 0;
    checked_sum += (packet.seq_no+packet.ack_no);
    for(int i=0;i<MAX_DATA_SIZE;i++)
        checked_sum+= packet.data[i];
    
    return (checked_sum==packet.checksum);

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

			

void set_init_time(PacketArrayNode& packetNode){
    struct timeval tp;
    gettimeofday(&tp, NULL);
    long int ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
    packetNode.send_time = ms;
}










#endif //__LIB.H___