// Server side implementation of UDP client-server model

#include "lib.h"

//Initial string buffer to read data from stdin
string out_buffer="";

//Mutex for blocking sender thread before completing user's input 
pthread_mutex_t send_mutex;


//termination flag
bool _terminate_=false;

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

	//Sends inital packet 20 times
	//This way not needed to handshake
	//Since almost certainly any of the packet will
	//Arrive

	Packet init_packet;
	create_init_packet(init_packet);

	for(int i=0;i<20;i++)
		sendto(cli_sockfd, &init_packet,sizeof(Packet),0,(const struct sockaddr *) &servaddr,sizeof(servaddr));



	while(1){

		//Waits until stdin input enterd
		pthread_mutex_lock(&send_mutex);


		//parsers message into packetlist
		int packet_count = msg_to_packet(out_buffer,out_window);
		
		for(int i=0;i<packet_count;i++){

			//selective repeat mechanism it blocks sending packages
			//greater than window
			while(1){
				if((SENT_LAST-ACKED_MIN <= WIN_SIZE))
					break;
				
				sleep(0.001);
			}


			
			//adds sending time to each sending packet
			set_init_time(out_window[SENT_LAST]);
			sendto(cli_sockfd, &(out_window[SENT_LAST+1].packet),sizeof(Packet),0,(const struct sockaddr *) &servaddr,sizeof(servaddr));
			SENT_LAST++;


			
			
		}
	}

}

void* client_receiver(void*){
	int printed_last = -1;
	Packet packet;
	socklen_t len_servaddr = sizeof(servaddr);
	
	while(1){


		recvfrom(cli_sockfd,&packet,sizeof(Packet),MSG_WAITALL,(struct sockaddr *) &servaddr,&len_servaddr);
		

		if(check_packet_checksum(packet) == false){
			continue;
		}

		//For data packets I assigned its ack no to -1 
		//To distinguish between data and ack packets
		if(packet.ack_no == -1){//DATA PACKET

			//Also initally made all packets seq_no=-2 
			//This way when a packet arrives program can detect is it duplicate
			//If not then program adds it to incoming package buffer
			if(in_window[packet.seq_no].packet.seq_no == -2){//ORIGINAL PACKET
				assign_packet(&(in_window[packet.seq_no].packet),&packet);
				
				//Sends ack packet with corresponding data packet seq_no
				sendto(cli_sockfd, (make_ack(packet.seq_no)),sizeof(Packet),0,(const struct sockaddr *) &servaddr,len_servaddr);
				
				//When a packet arrives I will update which packets will be printed
				//Since seq=4 packet may came before seq=3 then 
				int i=printed_last+1;
				while(in_window[i].packet.seq_no >= 0){
					cout<<in_window[i].packet.data;
					i++;
					printed_last++;
				}
			
			}
			else{
				sendto(cli_sockfd, (make_ack(packet.seq_no)),sizeof(Packet),0,(const struct sockaddr *) &servaddr,len_servaddr);
			}
		
		}

		//If ACK packet arrived
		else{//ACK PACKET

			//If already ACKED packet's ack came, DUP_ACK
			//Program discards that
			if(out_window[packet.ack_no].is_acked == 1){//DUP ACKED

				continue;
			}
			else{
				out_window[packet.ack_no].is_acked = 1; //ORIGINAL ACK
			}
			

		}

	}
}

void* stdin_reader(void*){
	

	//Auxilary tempory string for getting message input
	string aux;
	string BYE="BYE";


	while(1){


		

		getline(cin,aux);
		

		if(aux == "BYE"){
			//some exit code
			_terminate_=true;
			//pthread_mutex_unlock(&send_mutex);
			break;
		}
		//Since my implementation fills packet's char buffer's unused parts
		//with zeros I needed to add this
		if(aux.length() > MAX_DATA_SIZE-2) aux+="\n";

		

		out_buffer=aux;
		//Getting input from user completed
		//stdin_reader thread can release releated mutex 
		pthread_mutex_unlock(&send_mutex);

	}

}

void* time_out(void*){

	struct timeval tp;
	long long int curr_time_ms;
	//To implement selective repeat I need to keep acked min packet
	//Below flag helps for it
	bool acked_min_flag;


	while(1){
		

		acked_min_flag = true;

		for(int i=0;i<MAX_PACKET_NUM;i++){
			

			//In out_window (outgoing packet buffer) to mark which elements
			//Empty in other words not sent
			//Initialize them with -1
			//If a packet is acked then is_acked became 1
			//This way I will skip not sent or already acked packets
			if(out_window[i].is_acked==1 || out_window[i].is_acked == -1){
				
				continue;
			}

			//Below code finds min_acked packet
			if(out_window[i].is_acked == 0 && i-1 > ACKED_MIN && acked_min_flag){
				ACKED_MIN = i-1;
				acked_min_flag=false;
			}
			//Below for calculating timeouts
			//And sending timedout packets again
			gettimeofday(&tp, NULL);
			curr_time_ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
			
			if(curr_time_ms-out_window[i].send_time >= TIMEOUT){
			
				set_init_time(out_window[i]);
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
	pid_t th1,th2,th3,th4;

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
	
	//Initializing packets as not acked and
	//not received
	for(int i=0;i<MAX_PACKET_NUM;i++){
		out_window[i].is_acked=-1;
		in_window[i].packet.seq_no=-2;

	}
	



	pthread_mutex_lock(&send_mutex);

    th1 = pthread_create(&stdin_reader_th,NULL,&stdin_reader,NULL);
    th2 = pthread_create(&client_sender_th,NULL,&client_sender,NULL);
	th3 = pthread_create(&client_receiver_th,NULL,&client_receiver,NULL);
	th4 = pthread_create(&time_out_th,NULL,&time_out,NULL);
	

	while(!_terminate_);
	
	//Killing other threads
	pthread_cancel(th1);
	pthread_cancel(th2);
	pthread_cancel(th3);
	pthread_cancel(th4);
	
	close(cli_sockfd);


}
