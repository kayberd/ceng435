// Server side implementation of UDP client-server model
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include "lib.h"
#include <time.h>


//#define SERV_PORT 1881
//#define CLI_PORT  1938
#define MAX_PACKET_NUM 1000

// Driver code

//pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
//pthread_mutex_t inp_buffer_write_lock = PTHREAD_MUTEX_INITIALIZER;

char* inp_buffer;
char* out_buffer;

//int send_sockfd;
int cli_sockfd;
struct sockaddr_in servaddr, cliaddr;
PacketArrayNode inp_window[MAX_PACKET_NUM];
PacketArrayNode out_window[MAX_PACKET_NUM];

int SERV_PORT;
int CLI_PORT;

void client_sender(){
		
	/*
		Şu an dümdüz inputu yolluyor sender.
	*/
	printf("Welcome to CHATWORK435 !!!\n");
	printf("Type 'BYE' to quit, press 'ENTER' to send\n");

	//FILE* fp = fopen("error.txt","w"); 
    //printf()
	while(1){
		out_buffer=read_stdin();
		sleep(0.1);
		int packet_count = msg_to_packet(out_buffer,out_window);

		
		for(int i=0;i<packet_count;i++){
			//fprintf(fp,"test31\n");
			//fprintf(fp,"%c",out_window[i].packet.data[0]);	
			//print_packet(fp,&(out_window[i].packet));
			//sendto(sockfd,(char*)hello,strlen(hello),0,(const struct sockaddr*)&servaddr,sizeof(servaddr));
			sendto(cli_sockfd, &(out_window[i].packet),sizeof(Packet),0,(const struct sockaddr *) &servaddr,sizeof(servaddr));
			//sleep(0.2);
		}
	}
	//printf("Hello message sent.\n");
}

void client_receiver(){
	char* msg;
	Packet packet;
	socklen_t len_cliaddr = sizeof(cliaddr);
	//FILE* fp = fopen("server.txt","w");
	
	while(1){
		
		//Packet packet;
		//packet.data[0] = 'b';
		//printf("31");
		//recvfrom(sockfd, (char *)buffer,20,MSG_WAITALL, ( struct sockaddr *) &cliaddr,&len_cliaddr);
		recvfrom(cli_sockfd, (Packet *)&packet,sizeof(Packet),MSG_WAITALL,(struct sockaddr *) &cliaddr,&len_cliaddr);
		//print_packet(stdout,&packet);
		//sleep(0.1);
		//printf("Her>>");
		//fflush(stdout);
		print_msg(stdout,packet_to_msg(&packet));
		//fflush(stdout);
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
	//fclose(fp);
}

void stdin_reader(){
	



    //printf("%s",out_buffer);

	;


}

void time_out(){
	;
}

int main(int argc,char** argv) {

	char* SERVER_IP = argv[1];
	SERV_PORT = atoi(argv[2]);
	CLI_PORT = atoi(argv[3]);

	pthread_t client_sender_th,client_receiver_th,stdin_reader_th;
	// Creating socket file descriptor
	if ( (cli_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("Socket Creation Failed");
		exit(EXIT_FAILURE);
	}
	
    
	
	memset(&servaddr, 0, sizeof(servaddr));
	memset(&cliaddr, 0, sizeof(cliaddr));
	
	// Filling server information
	servaddr.sin_family = AF_INET; // IPv4
	servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);
	servaddr.sin_port = htons(SERV_PORT);

	cliaddr.sin_family = AF_INET; // IPv4
	cliaddr.sin_addr.s_addr = inet_addr(SERVER_IP);
	cliaddr.sin_port = htons(CLI_PORT);

	
	// Bind the socket with the server address
	if ( bind(cli_sockfd, (const struct sockaddr *)&cliaddr,sizeof(cliaddr)) < 0 )
	{
		perror("Bind Failed");
		exit(EXIT_FAILURE);
	}
	

    //pthread_create(&stdin_reader_th,NULL,(void*)stdin_reader,NULL);
	//pthread_join(stdin_reader_th,NULL);
    pthread_create(&client_sender_th,NULL,(void*)client_sender,NULL);
	pthread_create(&client_receiver_th,NULL,(void*)client_receiver,NULL);
	

	pthread_join(client_sender_th,NULL);
	pthread_join(client_receiver_th,NULL);
	

	


}
