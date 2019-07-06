/* Nicholas Freeman
 * Date: June 2nd 2019
 * Description: This file implements Dijkstra's link state algorithm to find the shortest route between any two nodes in a 4x4 matrix.
 *				each value in the matrix is representative of a different distances between two nodes, and all nodes are connected. Nodes
 * 				may be initialized to any value throughout the duration of the file. Currently only works for a 4x4 matrix, but may be
 *				modified for any NxN matrix | N >= 0. Requires an input cost text file and a data text file. 
*/

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/time.h>
#include <limits.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdbool.h>

typedef struct{
	char name[50];
	char ip[50];
	int port;
} MACHINE;

#define MIN_W 10
#define MAX_W 20

pthread_mutex_t lock;

int mySock;
int in_data[3]; //array to hold first host, second host, weight
int out_data[3]; //array to hold first host, second host, weight
MACHINE hosts[100];
int N = 4;
int cost[100][100];
int id;
int myPort;

void send_data();
int receive_data(int port);
void user_input_cost();      // Implement Dijkstra's 

void parse_files(FILE* f1, FILE* f2);
void *updateThreads();
void *LS();
// Input:
// <./a.out> <ID(Integer Value)> <N(Integer Value)> <costs.txt(text file)> <dats.txt(text file)>

// Main thread (Thread 1)
int main(int argc, char* argv[]){
	if (argc != 5)
		printf ("Usage: %s <id> <n_machines> <costs_file> <hosts_file> \n",argv[0]);

	// Scan data
	sscanf(argv[1],"%d",&id);
	sscanf(argv[2],"%d",&N);

	FILE *f1;
	f1 = fopen(argv[3], "r");
	FILE *f2;
	f2 = fopen(argv[4], "r");

	pthread_mutex_init(&lock, NULL);

	parse_files(f1, f2);
	myPort = hosts[id].port;

	struct sockaddr_in myAddr;
	socklen_t addr_size;

	//UDP Stuff
	myAddr.sin_family = AF_INET;
	myAddr.sin_port = htons ((short)myPort);
	myAddr.sin_addr.s_addr = htonl (INADDR_ANY);
	memset ((char *)myAddr.sin_zero, '\0', sizeof (myAddr.sin_zero));
	addr_size = sizeof (myAddr);

	//Create a socket
	if ((mySock = socket (AF_INET, SOCK_DGRAM, 0)) < 0){
		printf ("SOCKET ERROR\n");
		return 1;
	}

	//Bind
	if (bind(mySock, (struct sockaddr *)&myAddr, sizeof(myAddr)) != 0){
		printf ("BIND ERROR\n");
		return 1;
	}

	//Initialize threads
	pthread_t thread2;
	pthread_create(&thread2, NULL, updateThreads, NULL);

	pthread_t thread3;
	pthread_create(&thread3, NULL, LS, NULL);

	int i;
	for(i = 0; i < 3;i++){
		user_input_cost();
		sleep(10);
	}
}

void parse_files(FILE* f1, FILE* f_hosts){
	printf("Cost file recieved...\n");
	int n, i, j, tempV;
	for(i = 0; i < N; i++){
		for(j = 0; j < N; j++){
			if (fscanf(f1,"%d",&cost[i][j]) != 1)
				break;
			printf("%d ", cost[i][j]);
		}
		printf("\n");
	}

	printf("Data file received...\n");
	for(i = 0; i < N; i++){
		if (fscanf(f_hosts,"%s %s %d", &(hosts[i].name), &(hosts[i].ip), &(hosts[i].port)) < 1)
			break;
		printf("%s %s %d \n",(hosts[i].name), (hosts[i].ip), (hosts[i].port));
	}
	return;
}

// Thread 2
void *updateThreads(){
	int i, j;
	double tempVar = 0;
	while(1){
		receive_data(myPort);
		// recvfrom (mySock, &in_data, sizeof(in_data), 0, NULL, NULL);
		printf("Received Update!\n");

		int host1 = ntohl(in_data[0]);
		int host2 = ntohl(in_data[1]);
		int weight = ntohl(in_data[2]);

		pthread_mutex_lock(&lock);
		cost[host1][host2] = weight;
		cost[host2][host1] = weight;

		for(i = 0; i < N; i++){
			for(j = 0; j < N; j++){
				printf("%d ", cost[i][j]);
			}
			printf("\n");
		}
		pthread_mutex_unlock(&lock);
	}
}

// Thread 3
void *LS(){
	time_t last_update;
	last_update = time(NULL);
	while(1){
		int threshold = rand()%(MAX_W - MIN_W) + MIN_W;
		if ((time(NULL) - last_update) > threshold){
      // Implement Dijkstra's 
			int i, source, count, u, v;
			int tmp_costs[N][N];
			int dist[N];
			int visited[N];

			pthread_mutex_lock(&lock);
			for(source = 0; source < N; source++){
				// Init values
				for (i = 0; i < N; i++)
					dist[i] = INT_MAX, visited[i] = 0;

        // Distance to self is 0 (So diagnals are 0 in cost matrix)
				dist[source] = 0;

				for (count = 0; count < N-1; count++){
					u = minDist(dist, visited);
					visited[u] = 1;

					for (v = 0; v < N; v++)
						if (visited[v]==0 && cost[u][v] && dist[u] != INT_MAX && dist[u]+cost[u][v] < dist[v])
						dist[v] = dist[u] + cost[u][v];
				}

				printf("Cost Array %d: ",source);
				for (i = 0; i < N; i++)
				{
					printf("%d ",dist[i]);
					tmp_costs[source][i] = dist[i];
					tmp_costs[i][source] = dist[i];
				}
				printf("\n");
			}
			pthread_mutex_unlock(&lock);
			last_update = time(NULL);
		}
	}
}

int minDist(int dist[], int visited[]){
   int min = INT_MAX, min_index;
   int v;

   for (v = 0; v < N; v++)
     if (visited[v] == 0 && dist[v] < min)
         min = dist[v], min_index = v;

   return min_index;
}

void send_data(){
	int sock, i;
	double temp2 = 0;
	struct sockaddr_in destAddr[N];
	socklen_t addr_size[N];

	for (i = 0; i < N; i++){
		destAddr[i].sin_family = AF_INET;
		destAddr[i].sin_port = htons (hosts[i].port);
		inet_pton (AF_INET, hosts[i].ip, &destAddr[i].sin_addr.s_addr);
		memset (destAddr[i].sin_zero, '\0', sizeof (destAddr[i].sin_zero));
		addr_size[i] = sizeof destAddr[i];
	}

	sock = socket(PF_INET, SOCK_DGRAM, 0);

	for (i = 0; i < N; i++)
		if (i != id)
			sendto (sock, &out_data, sizeof(out_data), 0, (struct sockaddr *)&(destAddr[i]), addr_size[i]);
}

int receive_data(int port){
	int nBytes = recvfrom (mySock, &in_data, sizeof(in_data), 0, NULL, NULL);
	printf("Updated\n");
	return 0;
}

void user_input_cost(){
	int stuff;
	int neighbor;
	double temp1 = 0;
	int newDistance;

	printf("Updated cost from a node %d: <neighbor> <new cost>:\n", id);

	scanf("%d %d", &neighbor, &newDistance);

	pthread_mutex_lock(&lock);
	cost[id][neighbor] = newDistance;
	cost[neighbor][id] = newDistance;
	out_data[0] = htonl(id);
	out_data[1] = htonl(neighbor);
	out_data[2] = htonl(newDistance);
	send_data();

	printf("Updated Matrix:\n");
	
	int i, j;
	for(i = 0; i < N; i++){
		for(j = 0; j < N; j++){ 
		    printf("%d ", cost[i][j]);
		}
		printf("\n");
	}
	pthread_mutex_unlock(&lock);
}

