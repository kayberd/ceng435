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
#define WINDOW_SIZE 4
#define MAX_PACKET_NUM 1000
#define TIMEOUT 1000 //timeout currently 1000ms 1s

// Driver code

//pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
//pthread_mutex_t inp_buffer_write_lock = PTHREAD_MUTEX_INITIALIZER;

char* inp_buffer;
char* out_buffer;

//int send_sockfd;
int cli_sockfd;
struct sockaddr_in servaddr, cliaddr;
PacketArrayNode in_window[MAX_PACKET_NUM];
PacketArrayNode out_window[MAX_PACKET_NUM];

int SERV_PORT;
int CLI_PORT;

unsigned int ACKED_MIN=-1;
unsigned int SENT_LAST=-1;
unsigned int SEQ_NUM=0;


void client_sender(){
		
	printf("Welcome to CHATWORK435 !!!\n");
	printf("Type 'BYE' to quit, press 'ENTER' to send\n");


	while(1){

		out_buffer=read_stdin();
		
		int packet_count = msg_to_packet(out_buffer,out_window);
		
		for(int i=1;i<packet_count+1;i++){

			// TODO:   ADD mutex or cond var here
			//Busy wait currently
			while(!(SENT_LAST-ACKED_MIN < WIN_SIZE)) sleep(0.1);

			printf("%d\n",ACKED_MIN);
			sendto(cli_sockfd, &(out_window[ACKED_MIN+i].packet),sizeof(Packet),0,(const struct sockaddr *) &servaddr,sizeof(servaddr));
			//SEQ_NUM++;
			SENT_LAST = SEQ_NUM;
		}
	}

}

void client_receiver(){
	char* msg;
	Packet packet;
	socklen_t len_servaddr = sizeof(servaddr);
	long unsigned int check_sum;
	
	while(1){
		


		recvfrom(cli_sockfd,&packet,sizeof(Packet),MSG_WAITALL,(struct sockaddr *) &servaddr,&len_servaddr);
		

		/*if(check_packet_checksum(&packet,&check_sum)){
			printf("Correct check_sum checked sum = %lu\n",check_sum);
		}
		else{
			printf("Incorrect checksum\n");
		}*/
		if(packet.ack_no == -1){
			if(assign_packet(&(in_window[packet.seq_no].packet),&packet) == -1)
				fprintf(stdin,"Packet assign failed \n");
			
			//print_msg(stdout,packet_to_msg(&packet));
			print_packet(stdout,&(in_window[packet.seq_no].packet));
			sendto(cli_sockfd, (make_ack(packet.seq_no)),sizeof(Packet),0,(const struct sockaddr *) &servaddr,len_servaddr);

		
		}
		else{

			in_window[packet.seq_no].is_acked = 1;

			if(packet.ack_no == ACKED_MIN+1){
				for(int i=1;i<WIN_SIZE+1;i++){
					if(in_window[ACKED_MIN+i].is_acked == 1){
						ACKED_MIN++;
					}
					else{
						break;
					}
				}
			}
		}

	}
}

void stdin_reader(){
	




	;


}

void time_out(){

	struct timeval tp;
	long int curr_time_ms;
	
	while(1){
		sleep(15);
		for(int i=ACKED_MIN;i<SENT_LAST+1;i++){
			if(out_window[i].is_acked==1)
				continue;
			gettimeofday(&tp, NULL);
			curr_time_ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
			if(out_window[i].is_acked==0 && curr_time_ms-out_window[i].send_time >= TIMEOUT){
				sendto(cli_sockfd, &(out_window[i].packet),sizeof(Packet),0,(const struct sockaddr *) &servaddr,sizeof(servaddr));
			}
		}
	}
}

int main(int argc,char** argv) {

	char* SERVER_IP = argv[1];
	SERV_PORT = atoi(argv[2]);
	CLI_PORT = atoi(argv[3]);

	pthread_t client_sender_th,client_receiver_th,stdin_reader_th,time_out_th;
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
	//pthread_create(&time_out_th,NULL,(void*)time_out,NULL);
	

	pthread_join(client_sender_th,NULL);
	pthread_join(client_receiver_th,NULL);
	//pthread_join(time_out_th,NULL);
	


}
