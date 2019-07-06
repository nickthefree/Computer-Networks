/*
* Nicholas Freeman
* 4/18/2019
* Description: This program is a simple UDP client that sends a copy of a src.dat file 1024 bits at a time. This file is received by a server on port 5000 via UDP connection.
*/

/**************************
UDP socket example, client
**************************/
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SIZE 1024

struct HEADER {
    int seq_ack;
    int len;
    int cksum;
};

struct PACKET {
    struct HEADER head;
    char data[10];
};

int main(){
    int sock;
    struct sockaddr_in server_addr;
    struct hostent *host;
    char send_data[SIZE];
    socklen_t addr_len;
    host = (struct hostent *)gethostbyname((char *)"127.0.0.1");
    FILE *f2;
    f2 = fopen("src.dat", "rb");
    if (f2 == NULL){
	printf("Cannot open file src.dat \\n");
	exit(0);
    }
    // open socket
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
	perror("socket");
	exit(1);
    }
    // set address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5000);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    int x;
    //send src.dat to server
    while (x = fread(send_data, 1, SIZE, f2)){
	sendto(sock, send_data, x, 0,(struct sockaddr *)&server_addr, sizeof(struct sockaddr));
    }
}
