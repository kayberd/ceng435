// Server side implementation of UDP client-server model

#include "lib.h"




//pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
//pthread_mutex_t inp_buffer_write_lock = PTHREAD_MUTEX_INITIALIZER;

string out_buffer="";

pthread_mutex_t out_buffer_mx;
pthread_mutex_t send_mutex;
//pthread_mutex_t init_packet_mx;
pthread_mutex_t out_window_mx;
pthread_mutex_t acked_min_mx;
pthread_mutex_t not_bye_mx;

bool not_bye=true;

//int send_sockfd;
int serv_sockfd;
struct sockaddr_in servaddr, cliaddr;
PacketArrayNode in_window[MAX_PACKET_NUM];
PacketArrayNode out_window[MAX_PACKET_NUM];

int SERV_PORT;
int CLI_PORT;

int ACKED_MIN=-1;
int SENT_LAST=-1;
int SEQ_NUM=0;


void* server_sender(void*){



	printf("Welcome to CHATWORK435 !!!\n");
	printf("Type 'BYE' to quit, press 'ENTER' to send\n");


	
	

	
	while(1){/*
		pthread_mutex_lock(&not_bye_mx);
		if(not_bye == false){
			pthread_mutex_unlock(&not_bye_mx);
			break;
		}
		pthread_mutex_unlock(&not_bye_mx);*/

		pthread_mutex_lock(&send_mutex);
		pthread_mutex_lock(&out_buffer_mx);
		int packet_count = msg_to_packet(out_buffer,out_window);
		pthread_mutex_unlock(&out_buffer_mx);
		
		for(int i=0;i<packet_count;i++){

			// TODO:   ADD mutex or cond var here
			//Busy wait currently
			//printf("SENT_LAST:%d\n",SENT_LAST);
			//sleep(0.00001);
			int k=0;
			while(1){
				k++;
				pthread_mutex_lock(&acked_min_mx);
				if((SENT_LAST-ACKED_MIN <= WIN_SIZE)){
					pthread_mutex_unlock(&acked_min_mx);
					break;
				} 
				if(k==100000){
					printf("Waiting ACK:%d\n",ACKED_MIN);
					k=0;
					//print_flag_on=0;
				}

				//printf("Resending packet:%d\n",ACKED_MIN+1);
				//sendto(cli_sockfd, &(out_window[ACKED_MIN+1].packet),sizeof(Packet),0,(const struct sockaddr *) &servaddr,sizeof(servaddr));

				pthread_mutex_unlock(&acked_min_mx);
				sleep(0.01);
			}
			//print_flag_on=1;


			
			
			//printf("%d\n",SENT_LAST);
			pthread_mutex_lock(&out_window_mx);
			set_init_time(out_window[SENT_LAST]);
			sendto(serv_sockfd, &(out_window[SENT_LAST+1].packet),sizeof(Packet),0,(const struct sockaddr *) &cliaddr,sizeof(cliaddr));
			SENT_LAST++;

			pthread_mutex_unlock(&out_window_mx);
			//print_packet(stdout,&(out_window[SENT_LAST].packet));
			//SEQ_NUM++;
			
			//SENT_LAST = SEQ_NUM;
			
		}
	}

}

void* server_receiver(void*){

	int printed_last = -1;
	string msg;
	Packet packet;
	socklen_t len_cliaddr = sizeof(cliaddr);
	long unsigned int check_sum;
	
	while(1){

		/*pthread_mutex_lock(&not_bye_mx);
		if(not_bye == false){
			pthread_mutex_unlock(&not_bye_mx);
			break;
		}
		pthread_mutex_unlock(&not_bye_mx);*/

		recvfrom(serv_sockfd,&packet,sizeof(Packet),MSG_WAITALL,(struct sockaddr *) &cliaddr,&len_cliaddr);
		
		//print_packet(stdout,&(in_window[packet.seq_no].packet));

		if(check_packet_checksum(packet) == false){
			sleep(0.01);
			continue;
		}

		if(packet.ack_no == -1){//DATA PACKET
			
			if(in_window[packet.seq_no].packet.seq_no == -2){//ORIGINAL PACKET
				if(assign_packet(&(in_window[packet.seq_no].packet),&packet) == -1)
					fprintf(stdin,"Packet assign failed \n");
				
				//print_msg(stdout,packet_to_msg(&packet));
				//print_packet(stdout,&(in_window[packet.seq_no].packet));
				int i=printed_last+1;
				while(in_window[i].packet.seq_no >= 0){
					cout<<in_window[i].packet.data;
					i++;
					printed_last++;
				}
	


				sendto(serv_sockfd, (make_ack(packet.seq_no)),sizeof(Packet),0,(const struct sockaddr *) &cliaddr,len_cliaddr);
			}
			else{
				sendto(serv_sockfd, (make_ack(packet.seq_no)),sizeof(Packet),0,(const struct sockaddr *) &cliaddr,len_cliaddr);
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


		}

	}
}

void* stdin_reader(void*){
	

	while(1){
/*
		pthread_mutex_lock(&not_bye_mx);
		if(not_bye == false){
			pthread_mutex_unlock(&not_bye_mx);
			break;
		}
		pthread_mutex_unlock(&not_bye_mx);
*/
		string aux;

		getline(cin,aux);
		if(aux.length() > MAX_DATA_SIZE-2 )aux+="\n";
		pthread_mutex_unlock(&send_mutex);
		if(aux == "BYE"){
			//some exit code
			pthread_mutex_lock(&not_bye_mx);
			not_bye=false;
			pthread_mutex_unlock(&not_bye_mx);
		}

		pthread_mutex_lock(&out_buffer_mx);
		out_buffer=aux;
		pthread_mutex_unlock(&out_buffer_mx);

	}

}

void* time_out(void*){

	struct timeval tp;
	long int curr_time_ms;
	


	while(1){
/*
		pthread_mutex_lock(&not_bye_mx);
		if(not_bye == false){
			pthread_mutex_unlock(&not_bye_mx);
			break;
		}	
		pthread_mutex_unlock(&not_bye_mx);
		
*/		
		pthread_mutex_lock(&out_window_mx);
		int printed_last=-1,t;
		for(int i=0;i<MAX_PACKET_NUM;i++){
			
			if(out_window[i].is_acked==1 || out_window[i].is_acked == -1){
				
				continue;
			}
			
			gettimeofday(&tp, NULL);
			curr_time_ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
			
			if(curr_time_ms-out_window[i].send_time >= TIMEOUT){
				
				pthread_mutex_lock(&acked_min_mx);
				if(out_window[i].packet.seq_no-1 > ACKED_MIN)
					ACKED_MIN = out_window[i].packet.seq_no-1;
				pthread_mutex_unlock(&acked_min_mx);				

				set_init_time(out_window[i]);
				sendto(serv_sockfd, &(out_window[i].packet),sizeof(Packet),0,(const struct sockaddr *) &cliaddr,sizeof(cliaddr));
			}

			
			
		}
		pthread_mutex_unlock(&out_window_mx);
		sleep(0.001);

	}
	
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
	//memset(&cliaddr, 0, sizeof(cliaddr));
	
	// Filling server information
	servaddr.sin_family = AF_INET; // IPv4
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(SERV_PORT);

	cliaddr.sin_family = AF_INET; // IPv4
	cliaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	cliaddr.sin_port = htons(CLI_PORT);

	
	// Bind the socket with the server address
	if ( bind(serv_sockfd, (const struct sockaddr *)&servaddr,sizeof(servaddr)) < 0 )
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
    pthread_create(&server_sender_th,NULL,&server_sender,NULL);
	pthread_create(&server_receiver_th,NULL,&server_receiver,NULL);
	pthread_create(&time_out_th,NULL,&time_out,NULL);
	

	pthread_join(server_sender_th,NULL);
	pthread_join(server_receiver_th,NULL);
	pthread_join(stdin_reader_th,NULL);
	pthread_join(time_out_th,NULL);
	


}
