/* 
* Nicholas Freeman
* 4/25/2019
* Description: This program runs a tcp server with multithreading to support multiple connections (Also utilizes mutexes for synchronization.)
*/

/**************************
*  TCP socket example, server
***************************/
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>

#define MAXCON 2

FILE *f1; 
void *threadwrite(void *arg);
pthread_t tids[MAXCON];
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int main(){
    int size;
    int sock,  true = 1, i;
    struct sockaddr_in server_addr, client_addr;
    //Open socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
	perror("Socket");
	exit(1);
    }
    //Set address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5000);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    //Bind socket
    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1){
	perror("Unable to bind");
	exit(1);
    }
    printf("\nTCPServer Waiting for client on port 5000: \t");
    //Listen
    if (listen(sock, 5) == -1){
	perror("Listen");
	exit(1);
    }
    
    for(i=0; i <= MAXCON ; i++){
	int *CON = malloc(sizeof(int));
	*CON = accept(sock, (struct sockaddr *)&client_addr, (socklen_t *)&size);
	pthread_create(&tids, NULL, threadwrite, CON);    
    }
    fclose(f1);
    close(sock);
    return 0;
}

void *threadwrite(void *arg){
    pthread_mutex_lock(&lock);
    int bytes_recieved;
    int connfd = *(int*)arg;
    char  recv_data[1024];

    while((bytes_recieved = recv(connfd, recv_data, 1024, 0)) > 0){    

	f1 = fopen("dest.dat", "a");
	
	if (!feof(f1)){
	    fwrite(recv_data, 1, bytes_recieved, f1);
	}
	else{
	    printf("\n Could Not open dest.dat file");
	}
	fflush(f1);  //changed to fflush
    }
    pthread_mutex_unlock(&lock);    

    
}


