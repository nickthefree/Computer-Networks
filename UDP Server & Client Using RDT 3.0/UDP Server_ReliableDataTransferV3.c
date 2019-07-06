// Nicholas Freeman
// May 2nd 2019
// UDP server
// Uses reliable data transfer 3.0 (rdt3.0) behavior to send a recieve a file (namely src.dat) and writes a copy to dest.dat.
// Run using syntax:
// <./out file> <file name> <port number>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

// <file name> <port number>
struct HEADER{
	int seq_ack;	
	int len;	
	int cksum;	
};

struct PACKET{
	struct HEADER head;
	char data[10];
};

int csum(char *buff, int buffsize){
	int cs = 0;
	int i;
	for(i = 0; i < buffsize; i++){
		cs^=buff[i];
	}
	return cs;
}

int main(int argc, char *argv[]){
	struct sockaddr_in serverAddr, clientAddr;
	struct sockaddr_storage serverStorage;
	int sock, nBytes, chk, f = -1;
	socklen_t addr_size, client_addr_size;
	char buff[10];
	struct PACKET p;
	FILE *dest;
	int status = 0;
	struct HEADER ack;
	srand(time(NULL));



	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(atoi(argv[1]));
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	memset((char*)serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));
	addr_size = sizeof(serverStorage);

	//Create socket
	if((sock=socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		perror("Socket error\n");
		return 1;
	}

	//Bind
	if(bind(sock,(struct sockaddr *)&serverAddr, sizeof(serverAddr)) != 0){
		perror("Bind error\n");
		return 1;
	}
	
	//open file

	dest = fopen("dest.dat", "wb");
	f = 1;

	ack.len = 0;
	ack.cksum = 0;
	ack.seq_ack = status;
	f = -1;

	while(f == -1){
		chk = rand() % 10;
		nBytes = recvfrom(sock,&p,sizeof(struct PACKET),0,(struct sockaddr *)&serverStorage, &addr_size);
		
		int sentChk = p.head.cksum;
		p.head.cksum = 0;
		int sentSeq = p.head.seq_ack;

		p.head.cksum = csum((char*)&p, sizeof(struct HEADER) + sizeof(p.data));

		if(p.head.len == 0){
			printf("Final packet of length 0 recieved\n");
			f = 1;
		}

		ack.seq_ack =! status;
		if(p.head.cksum != sentChk){
			perror("Wrong chksum\n");
			if(chk > 1)
				sendto(sock, &ack, sizeof(struct HEADER), 0, (struct sockaddr *)&serverStorage, addr_size);
		}
		else if(p.head.cksum == sentChk){
			printf("Correct chksum\n");
			if(p.head.seq_ack != status){
				perror("Wrong seqN\n");
				if(chk > 1)
					sendto(sock, &ack, sizeof(struct HEADER), 0, (struct sockaddr *)&serverStorage, addr_size);
			}
			else if(p.head.seq_ack == status){
				printf("Correct seqN\n");
				ack.seq_ack = status;
				fwrite(p.data, 1, p.head.len, dest);
				status = !status;
				if(chk > 1){
					sendto(sock, &ack, sizeof(struct HEADER), 0, (struct sockaddr *)&serverStorage, addr_size);
					printf("ACK sent\n");
				}
			}
		}
	}

	//Close files & Terminate Connection
	fclose(dest);
	close(sock);
	return 0;
}
