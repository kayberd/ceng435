// Server side implementation of UDP client-server model
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include "lib.h"
#include <time.h>

#define PORT	 3000
#define MAX_PACKET_NUM 10

// Driver code

//pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
//pthread_mutex_t inp_buffer_write_lock = PTHREAD_MUTEX_INITIALIZER;

char* inp_buffer;
char* out_buffer;

int sockfd;
struct sockaddr_in servaddr, cliaddr;
PacketArrayNode inp_window[MAX_PACKET_NUM];
PacketArrayNode out_window[MAX_PACKET_NUM];

void client_sender(){


		
	/*
		Şu an dümdüz inputu yolluyor sender.
	*/
	int packet_count = msg_to_packet(out_buffer,out_window);
	FILE* fp = fopen("error.txt","w"); 
    //printf()
	//while(1){
		for(int i=0;i<packet_count;i++){
			//fprintf(fp,"test31\n");
			//fprintf(fp,"%c",out_window[i].packet.data[0]);	
			//print_packet(fp,&(out_window[i].packet));
			//sendto(sockfd,(char*)hello,strlen(hello),0,(const struct sockaddr*)&servaddr,sizeof(servaddr));
			sendto(sockfd, &(out_window[i].packet),sizeof(Packet),0,(const struct sockaddr *) &servaddr,sizeof(servaddr));
			//sleep(0.2);
		}
	//}
	//printf("Hello message sent.\n");
}

void client_receiver(){
	char* msg;
	Packet packet;
    int len_serv_addr = sizeof(servaddr); //len is value/resuslt
	
	while(1){
		recvfrom(sockfd, (Packet *)&packet,sizeof(Packet),MSG_WAITALL,(struct sockaddr *) &servaddr,&len_serv_addr);
		packet_to_msg(&packet,msg);
		printf("%s",msg);
	}
}
void stdin_reader(){
	


	printf("Welcome to CHATWORK435 !!!\n");
	printf("Type 'BYE' to quit, press 'ENTER' to send\n");

	out_buffer=read_stdin();
    //printf("%s",out_buffer);

	


}
void time_out(){
	;
}
int main() {


	pthread_t client_sender_th,client_receiver_th,stdin_reader_th;
	// Creating socket file descriptor
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("Socket Creation Failed");
		exit(EXIT_FAILURE);
	}
    
	
	memset(&servaddr, 0, sizeof(servaddr));
	//memset(&cliaddr, 0, sizeof(cliaddr));
	
	// Filling server information
	servaddr.sin_family = AF_INET; // IPv4
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(PORT);
	
	// Bind the socket with the server address
	/*if ( bind(sockfd, (const struct sockaddr *)&servaddr,sizeof(servaddr)) < 0 )
	{
		perror("Bind Failed");
		exit(EXIT_FAILURE);
	}*/
	

    pthread_create(&stdin_reader_th,NULL,(void*)stdin_reader,NULL);
	pthread_join(stdin_reader_th,NULL);
    pthread_create(&client_sender_th,NULL,(void*)client_sender,NULL);
	//pthread_create(&client_receiver_th,NULL,(void*)client_receiver,NULL);
	

	pthread_join(client_sender_th,NULL);
	//pthread_join(client_receiver_th,NULL);
	

	


}
