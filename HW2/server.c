// Server side implementation of UDP client-server model
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include "lib.h"
#include <time.h>

#define PORT	 1881
#define MAX_PACKET_NUM 1000

// Driver code

//pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t inp_buffer_write_lock = PTHREAD_MUTEX_INITIALIZER;

char* inp_buffer;
char* out_buffer;

int sockfd;
struct sockaddr_in servaddr, cliaddr;
PacketArrayNode inp_window[MAX_PACKET_NUM];
PacketArrayNode out_window[MAX_PACKET_NUM];

void* server_sender(){


		
	/*
		Şu an dümdüz inputu yolluyor sender.
	*/
	//pthread_mutex_lock(&inp_buffer_write_lock);
	int packet_count = msg_to_packet(out_buffer,out_window); 
	int len_cliaddr = sizeof(cliaddr); //len is value/resuslt
	for(int i=0;i<packet_count;i++){
		sendto(sockfd, (Packet*)&(out_window[i].packet),sizeof(Packet),MSG_CONFIRM,(const struct sockaddr *) &cliaddr,len_cliaddr);
		sleep(0.2);
	}
	printf("Hello message sent.\n");
	while(1);
}

void* server_receiver(){
	char* msg;
	Packet packet;
	socklen_t len_cliaddr = sizeof(cliaddr);
	FILE* fp = fopen("server.txt","w");
	
	while(1){
		
		//Packet packet;
		//packet.data[0] = 'b';
		//printf("31");
		//recvfrom(sockfd, (char *)buffer,20,MSG_WAITALL, ( struct sockaddr *) &cliaddr,&len_cliaddr);
		recvfrom(sockfd, (Packet *)&packet,sizeof(Packet),MSG_WAITALL,(struct sockaddr *) &cliaddr,&len_cliaddr);
		//print_packet(stdout,&packet);
		//sleep(0.1);
		print_msg(stdout,packet_to_msg(&packet));
		//fflush(stdout);
		//fflush(stdout);
		//fflush(stdout);
		//fflush(stdout);
		//sleep(0.1);
		//sleep(0.3);
		//fclose(fp);
		//break;
		//fprintf(fp,"%s",msg);
		
		//while(1) printf("31");
		//sleep(31);
	}
	fclose(fp);
	
}
void* stdin_reader(){
	


	printf("Welcome to CHATWORK435 !!!\n");
	printf("Type 'BYE' to quit, press 'ENTER' to send\n");

	out_buffer=read_stdin();

	


}
void time_out(){
	;
}
int main() {


	pthread_t server_sender_th,server_receiver_th,stdin_reader_th;
	// Creating socket file descriptor
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("Socket Creation Failed");
		exit(EXIT_FAILURE);
	}
    
	
	memset(&servaddr, 0, sizeof(servaddr));
	memset(&cliaddr, 0, sizeof(cliaddr));
	
	// Filling server information
	servaddr.sin_family = AF_INET; // IPv4
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(PORT);
	
	// Bind the socket with the server address
	if ( bind(sockfd, (const struct sockaddr *)&servaddr,sizeof(servaddr)) < 0 )
	{
		perror("Bind Failed");
		exit(EXIT_FAILURE);
	}

	//pthread_create(&stdin_reader_th,NULL,stdin_reader,NULL);
	//pthread_join(stdin_reader_th,NULL);
	//pthread_create(&server_sender_th,NULL,server_sender,NULL);
	pthread_create(&server_receiver_th,NULL,server_receiver,NULL);
	//pthread_create(&stdin_reader_th,NULL,stdin_reader,NULL);

	//pthread_join(server_sender_th,NULL);
	pthread_join(server_receiver_th,NULL);
	//pthread_join(stdin_reader_th,NULL);

	


}
