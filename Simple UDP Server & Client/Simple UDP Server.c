/*
* Nicholas Freeman
* 4/18/2019
* Description: This program is the server side of a UDP server that receives a file src.dat and creates a copy dest.dat
*/


#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include<sys/types.h>
#include<sys/stat.h>
#include <fcntl.h>


#define SIZE 1024

/*********************\
*  main\
*********************/

int main()
{
    int sock;
    int bytes_read;
    char recv_data[SIZE];
    int y;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len;
    FILE *f1;
    f1 = fopen("dest.dat", "wb");

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
     	perror("Socket");
        exit(1);
    }
    // Set address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5000);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    //Bind socket to address
    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1){
     	perror("Bind");
        exit(1);
    }
    
    addr_len = sizeof(struct sockaddr);
    printf("UDPServer Waiting for client on port 5000\n");
    
    while (1){
     	//Receive from client
        bytes_read = recvfrom(sock, recv_data, SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
        fwrite(recv_data, 1, bytes_read, f1);
	if(bytes_read != SIZE)
	  break;
    }
    
    fclose(f1);
    return 0;
}
  
