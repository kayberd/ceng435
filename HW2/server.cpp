// Server side implementation of UDP client-server model
#include "lib.h"



//#define SERV_PORT 1881
//#define CLI_PORT  1938


// Driver code

//pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
//pthread_mutex_t inp_buffer_write_lock = PTHREAD_MUTEX_INITIALIZER;

string out_buffer;

pthread_mutex_t out_buffer_mx;
pthread_mutex_t send_mutex;
//pthread_mutex_t init_packet_mx;
pthread_mutex_t out_window_mx;


//int send_sockfd;
int serv_sockfd;
struct sockaddr_in servaddr, cliaddr;
PacketArrayNode in_window[MAX_PACKET_NUM];
PacketArrayNode out_window[MAX_PACKET_NUM];

int SERV_PORT;
int CLI_PORT;

unsigned int ACKED_MIN=-1;
unsigned int SENT_LAST=-1;
unsigned int SEQ_NUM=0;


void* server_sender(void*){
		
	printf("Welcome to CHATWORK435 !!!\n");
	printf("Type 'BYE' to quit, press 'ENTER' to send\n");
	int print_flag_on=1;
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
			sleep(0.00001);
			while(!(SENT_LAST-ACKED_MIN <= WIN_SIZE)){
				if(print_flag_on){
					printf("Waiting ACK:%d\n",ACKED_MIN);
					print_flag_on=0;
				}

				//printf("Resending packet:%d\n",ACKED_MIN+1);
				//sendto(serv_sockfd, &(out_window[ACKED_MIN+1].packet),sizeof(Packet),0,(const struct sockaddr *) &cliaddr,sizeof(cliaddr));

				sleep(0.1);
			}

			
			
			//printf("%d\n",SENT_LAST);

        	set_init_time(out_window[SENT_LAST]);
			sendto(serv_sockfd, &(out_window[SENT_LAST].packet),sizeof(Packet),0,(const struct sockaddr *) &cliaddr,sizeof(cliaddr));
			//print_packet(stdout,&(out_window[SENT_LAST].packet));
			//SEQ_NUM++;
			
			//SENT_LAST = SEQ_NUM;
			
		}
	}


}

void* server_receiver(void*){
	string msg;
	Packet packet;
	socklen_t len_cliaddr = sizeof(cliaddr);
	long unsigned int check_sum;
	
	while(1){
		

		
		recvfrom(serv_sockfd,&packet,sizeof(Packet),MSG_WAITALL,(struct sockaddr *) &cliaddr,&len_cliaddr);
		//print_packet(stdout,&packet);
		//print_packet(stdout,&packet);

		/*if(check_packet_checksum(&packet,&check_sum)){
			printf("Correct check_sum checked sum = %lu\n",check_sum);
		}
		else{
			printf("Incorrect checksum\n");
		}*/
		if(packet.ack_no == -1 && in_window[packet.seq_no].packet.seq_no == -2){
			if(assign_packet(&(in_window[packet.seq_no].packet),&packet) == -1)
				fprintf(stdin,"Packet assign failed \n");
			
			//print_msg(stdout,packet_to_msg(&packet));
			print_packet(stdout,&(in_window[packet.seq_no].packet));
			sendto(serv_sockfd, (make_ack(packet.seq_no)),sizeof(Packet),0,(const struct sockaddr *) &cliaddr,len_cliaddr);

		
		}
		else if(packet.ack_no == -2){
			;
		}
		else{

			pthread_mutex_lock(&out_window_mx);
			if(out_window[packet.ack_no].is_acked == 0){
				out_window[packet.ack_no].is_acked = 1;
				pthread_mutex_unlock(&out_window_mx);
				continue;
			}
			pthread_mutex_unlock(&out_window_mx);
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
		//pthread_mutex_lock(&out_window_mx);
		for(int i=0;i<MAX_PACKET_NUM;i++){
			if(out_window[i].is_acked==1 || out_window[i].is_acked == -1){
				pthread_mutex_unlock(&out_window_mx);
				continue;
			}
			
			gettimeofday(&tp, NULL);
			curr_time_ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
			
			if(out_window[i].is_acked==0 && curr_time_ms-out_window[i].send_time >= TIMEOUT){
				
				
				//print_packet(time,&(out_window[i].packet));
				//fprintf(time,"Init:%ld  Curr:%ld",out_window[i].send_time,curr_time_ms);
				
				set_init_time(out_window[i]);
				sendto(serv_sockfd, &(out_window[i].packet),sizeof(Packet),0,(const struct sockaddr *) &cliaddr,sizeof(cliaddr));
			}
			
		}
		//pthread_mutex_unlock(&out_window_mx);

		sleep(0.001);
		//sleep(1);
		//fprintf(time,"--------------------------\n");
	}
	//fclose(time);
}

int main(int argc,char** argv) {

	SERV_PORT = atoi(argv[1]);
	

	pthread_t server_sender_th,server_receiver_th,stdin_reader_th,time_out_th;
	// Creating socket file descriptor
	if ( (serv_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("Socket Creation Failed");
		exit(EXIT_FAILURE);
	}
	
    
	
	memset(&servaddr, 0, sizeof(servaddr));
	memset(&cliaddr, 0, sizeof(cliaddr));
	
	// Filling server information
	servaddr.sin_family = AF_INET; // IPv4
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(SERV_PORT);
/*
	cliaddr.sin_family = AF_INET; // IPv4
	cliaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	cliaddr.sin_port = htons(1938);
*/
	
	// Bind the socket with the server address
	if ( bind(serv_sockfd, (const struct sockaddr *)&servaddr,sizeof(servaddr)) < 0 )
	{
		perror("Bind Failed");
		exit(EXIT_FAILURE);
	}
	
	pthread_mutex_lock(&send_mutex);

	for(int i=0;i<MAX_PACKET_NUM;i++){
		out_window[i].is_acked=-1;
		in_window[i].packet.seq_no=-2;

	}
	

    pthread_create(&stdin_reader_th,NULL,&stdin_reader,NULL);
    pthread_create(&server_sender_th,NULL,&server_sender,NULL);
	pthread_create(&server_receiver_th,NULL,&server_receiver,NULL);
	pthread_create(&time_out_th,NULL,&time_out,NULL);
	

	pthread_join(server_sender_th,NULL);
	pthread_join(server_receiver_th,NULL);
	pthread_join(stdin_reader_th,NULL);
	pthread_join(time_out_th,NULL);
	


}
