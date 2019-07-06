// Nicholas Freeman
// UDP client that sends a file (src.dat) to UDP server utilizing rdt3.0 protocol. 

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>

// Run using the syntax:
// <./a.out> <file name> <port number> <127.0.0.1> <source> <destination>

struct HEADER{
	int seq_ack;	
	int len;	//data length
	int cksum;	//checksum
};

struct PACKET{
	struct HEADER head;
	char data[10];
};

int csum(char* buff, int buffsize){
	int cs = 0;
	int i;
	for(i= 0; i < buffsize; i++){
		cs^=buff[i];
	}
	return cs;
}

int main(int argc, char *argv[]){
	//Create neccesary data structures and variables
	int sock, portNum, nBytes, rv, i, chk;
	struct sockaddr_in serverAddr;
	socklen_t addr_size;
	struct PACKET p;
	FILE *f1;
	int state = 0;
	struct HEADER ack;
	struct timeval tv;
	fd_set readfds;
	srand(time(NULL));
	
	//Create server and Socket
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(atoi(argv[1]));
	inet_pton (AF_INET, argv[2], &serverAddr.sin_addr.s_addr);
	memset (serverAddr.sin_zero, '\0', sizeof (serverAddr.sin_zero));  
	addr_size = sizeof serverAddr;

	
	sock = socket (PF_INET, SOCK_DGRAM, 0);
	fcntl(sock, F_SETFL, O_NONBLOCK);

	f1 = fopen(argv[3], "rb");

	//Send contents
	while ((nBytes = fread(p.data,1,10,f1))){
		int f = -1;
		p.head.len = nBytes;
		p.head.seq_ack = state;
		p.head.cksum = 0;
						
		while(f == -1){
			//Random to choose correct checksum or 0
			chk = rand() % 10;
			if(chk <= 1){
				p.head.cksum=0;
			}
			if(chk > 1){
				p.head.cksum=csum((char*)&p, sizeof(struct HEADER) + sizeof(p.data));
			}

			// send packet
			if(chk > 1){
				sendto (sock, &p, sizeof(struct PACKET), 0, (struct sockaddr *)&serverAddr, addr_size);
				printf("Packet sent\n");
			}

			//Set timer
			FD_ZERO(&readfds);
			FD_SET(sock, &readfds);
			tv.tv_sec = 10;
			tv.tv_usec = 0;

			//Use select as timer
			rv = select(sock+1, &readfds, NULL, NULL, &tv);
			
			if(rv == 0){
				printf("No data received, timeout\n");
			}

			//There is data to be received
			else if(rv == 1){
				// receive ack
				int c = recvfrom(sock, &ack, sizeof(struct HEADER), 0, NULL, NULL);
				if(ack.seq_ack==state){
					printf("Correct ACK received\n");
					f=1;
					state = !state;
				}
			}
		}
	}

	//Make final packet
	p.head.len = 0;
	p.head.cksum = 0;
	p.head.seq_ack = state;
	p.data[0] = '\0';
	p.head.cksum = csum((char*)&p, sizeof(struct HEADER) + sizeof(p.data));

	for(i = 0; i < 3; i++){
		printf("Sending final packet %d/3 times\n", i+1);
		if(chk > 1){
			sendto(sock, &p, sizeof(struct HEADER) + 1, 0, (struct sockaddr *)&serverAddr, addr_size);
		}
		
		//Set timer
		FD_ZERO(&readfds);
		FD_SET(sock, &readfds);

		tv.tv_sec = 10;
		tv.tv_usec = 0;

		rv = select(sock+1, &readfds, NULL, NULL,  &tv);
		if(rv == 1){
			printf("Final ACK received\n");
			int c = recvfrom(sock, &ack, sizeof(struct HEADER), 0, NULL, NULL);
			break;
		}
		else{
			printf("No data received, timeout\n");
		}
	}

	//Close file and connection
	fclose(f1);
	close(sock);

	return 0;
}
