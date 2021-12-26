// Server side implementation of UDP client-server model

#include "lib.h"




//pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
//pthread_mutex_t inp_buffer_write_lock = PTHREAD_MUTEX_INITIALIZER;

string out_buffer="";

pthread_mutex_t out_buffer_mx;
pthread_mutex_t send_mutex;
pthread_mutex_t init_packet_mx;
pthread_mutex_t out_window_mx;
pthread_mutex_t acked_min_mx;

bool is_terminated_sender;
bool is_terminated_receiver;
bool is_terminated_stdin;
bool is_terminated_timeout;

//int send_sockfd;
int cli_sockfd;
struct sockaddr_in servaddr, cliaddr;
PacketArrayNode in_window[MAX_PACKET_NUM];
PacketArrayNode out_window[MAX_PACKET_NUM];

int SERV_PORT;
int CLI_PORT;

int ACKED_MIN=-1;
int SENT_LAST=-1;
int SEQ_NUM=0;


void* client_sender(void*){



	printf("Welcome to CHATWORK435 !!!\n");
	printf("Type 'BYE' to quit, press 'ENTER' to send\n");


	//pthread_mutex_lock(&init_packet_mx);
	Packet init_packet;
	init_packet.seq_no=0;
	for(int i=0;i<MAX_DATA_SIZE;i++) init_packet.data[i]=0;
	init_packet.ack_no=-2;
	init_packet.checksum=-2;
	
	sendto(cli_sockfd, &(init_packet),sizeof(Packet),MSG_CONFIRM,(const struct sockaddr *) &servaddr,sizeof(servaddr));
	//print_packet(stdout,&init_packet);

	
	//int print_flag_on=1;
	while(1){

		//out_buffer=read_stdin();
		pthread_mutex_lock(&send_mutex);
		pthread_mutex_lock(&out_buffer_mx);
		int packet_count = msg_to_packet(out_buffer,out_window);
		pthread_mutex_unlock(&out_buffer_mx);
		//pthread_mutex_unlock(&send_mutex);
		
		for(int i=0;i<packet_count;i++){

			// TODO:   ADD mutex or cond var here
			//Busy wait currently
			SENT_LAST++;
			//printf("SENT_LAST:%d\n",SENT_LAST);
			//sleep(0.00001);
			
			while(1){
				pthread_mutex_lock(&acked_min_mx);
				if((SENT_LAST-ACKED_MIN <= WIN_SIZE)){
					pthread_mutex_unlock(&acked_min_mx);
					break;
				} 
				if(true){
					printf("Waiting ACK:%d\n",ACKED_MIN);
					//print_flag_on=0;
				}

				//printf("Resending packet:%d\n",ACKED_MIN+1);
				//sendto(cli_sockfd, &(out_window[ACKED_MIN+1].packet),sizeof(Packet),0,(const struct sockaddr *) &servaddr,sizeof(servaddr));

				pthread_mutex_unlock(&acked_min_mx);
				sleep(1);
			}
			//print_flag_on=1;


			
			
			//printf("%d\n",SENT_LAST);
			pthread_mutex_lock(&out_window_mx);
			set_init_time(out_window[SENT_LAST]);
			sendto(cli_sockfd, &(out_window[SENT_LAST].packet),sizeof(Packet),0,(const struct sockaddr *) &servaddr,sizeof(servaddr));
			pthread_mutex_unlock(&out_window_mx);
			//print_packet(stdout,&(out_window[SENT_LAST].packet));
			//SEQ_NUM++;
			
			//SENT_LAST = SEQ_NUM;
			
		}
	}

}

void* client_receiver(void*){
	string msg;
	Packet packet;
	socklen_t len_servaddr = sizeof(servaddr);
	long unsigned int check_sum;
	
	while(1){
		


		recvfrom(cli_sockfd,&packet,sizeof(Packet),MSG_WAITALL,(struct sockaddr *) &servaddr,&len_servaddr);
		
		//print_packet(stdout,&(in_window[packet.seq_no].packet));

		/*if(check_packet_checksum(&packet,&check_sum)){
			printf("Correct check_sum checked sum = %lu\n",check_sum);
		}
		else{
			printf("Incorrect checksum\n");
		}*/
		if(packet.ack_no == -1){//DATA PACKET
			
			if(in_window[packet.seq_no].packet.seq_no == -2){//ORIGINAL PACKET
				if(assign_packet(&(in_window[packet.seq_no].packet),&packet) == -1)
					fprintf(stdin,"Packet assign failed \n");
				
				//print_msg(stdout,packet_to_msg(&packet));
				print_packet(stdout,&(in_window[packet.seq_no].packet));
				sendto(cli_sockfd, (make_ack(packet.seq_no)),sizeof(Packet),0,(const struct sockaddr *) &servaddr,len_servaddr);
			}
			else{
				sendto(cli_sockfd, (make_ack(packet.seq_no)),sizeof(Packet),0,(const struct sockaddr *) &servaddr,len_servaddr);
				//continue;//DUP PACKET DISCARDED
			}
		
		}
		
		else{//ACK PACKET
			
			pthread_mutex_lock(&out_window_mx);
			if(out_window[packet.ack_no].is_acked == 1){//DUP ACKED
				printf("DUP-ACKED:%d\n",packet.ack_no);

				pthread_mutex_unlock(&out_window_mx);
				continue;
			}
			else{
				out_window[packet.ack_no].is_acked = 1; //ORIGINAL ACK
			}
			
			pthread_mutex_unlock(&out_window_mx);
			printf("ACKED: %d\n",packet.ack_no);

			//print_packet(stdout,&packet);

			pthread_mutex_lock(&acked_min_mx);
			if(packet.ack_no == ACKED_MIN+1){//ack came in order
				int ACKED_LAST= ACKED_MIN;
				pthread_mutex_lock(&out_window_mx);
				for(int i=ACKED_MIN+1;i<WIN_SIZE+ACKED_LAST+1;i++){
					
					if(out_window[i].is_acked == 1)
						ACKED_MIN++;
					
					else
						break;					
				}
				pthread_mutex_unlock(&out_window_mx);
			}
			pthread_mutex_unlock(&acked_min_mx);
		}

	}
}

void* stdin_reader(void*){
	

	while(true){

		string aux;

		getline(cin,aux);
		pthread_mutex_unlock(&send_mutex);
		if(aux == "BYE"){
			//some exit code
			;
		}

		pthread_mutex_lock(&out_buffer_mx);
		out_buffer=aux;
		pthread_mutex_unlock(&out_buffer_mx);

	}

}

void* time_out(void*){

	struct timeval tp;
	long int curr_time_ms;
	
	//FILE* time=fopen("client_time.txt","w");


	while(1){
		//sleep(15);
		pthread_mutex_lock(&out_window_mx);

		for(int i=0;i<MAX_PACKET_NUM;i++){
			
			if(out_window[i].is_acked==1 || out_window[i].is_acked == -1){
				
				continue;
			}
			
			gettimeofday(&tp, NULL);
			curr_time_ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
			
			if(curr_time_ms-out_window[i].send_time >= TIMEOUT){
				
				
				//print_packet(time,&(out_window[i].packet));
				//fprintf(time,"Init:%ld  Curr:%ld",out_window[i].send_time,curr_time_ms);
				set_init_time(out_window[i]);
				sendto(cli_sockfd, &(out_window[i].packet),sizeof(Packet),0,(const struct sockaddr *) &servaddr,sizeof(servaddr));
			}
			
			
		}
		pthread_mutex_unlock(&out_window_mx);
		sleep(0.001);
		//sleep(1);
		//fprintf(time,"--------------------------\n");
	}
	//fclose(time);
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
	
	for(int i=0;i<MAX_PACKET_NUM;i++){
		out_window[i].is_acked=-1;
		in_window[i].packet.seq_no=-2;

	}
	


	pthread_mutex_lock(&send_mutex);

    pthread_create(&stdin_reader_th,NULL,&stdin_reader,NULL);
    pthread_create(&client_sender_th,NULL,&client_sender,NULL);
	pthread_create(&client_receiver_th,NULL,&client_receiver,NULL);
	pthread_create(&time_out_th,NULL,&time_out,NULL);
	

	pthread_join(client_sender_th,NULL);
	pthread_join(client_receiver_th,NULL);
	pthread_join(stdin_reader_th,NULL);
	pthread_join(time_out_th,NULL);
	


}
