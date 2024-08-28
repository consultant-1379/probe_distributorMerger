/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFLEN 65536

pthread_mutex_t stat_mutex = PTHREAD_MUTEX_INITIALIZER;

long bytes = 0;
long packets = 0;

void* monitoring_function(void* parameters) {

	while (1) {
		time_t time_now;
		time(&time_now);
		pthread_mutex_lock(&stat_mutex);
		long period_packets = packets;
		long period_bytes = bytes;
		packets = 0;
		bytes = 0;
		pthread_mutex_unlock(&stat_mutex);

		double packets_per_second = (period_packets > 0 ? (double)(period_packets / 10) : 0.0);
		double bytes_per_second = (period_bytes > 0 ? (double)(period_bytes / 10) : 0.0);
		double bits_per_second = bytes_per_second * 8;

		printf("packets=%d, packets/second=%f, bytes=%d, bytes/second=%f, Mbits/second=%f\n",
				period_packets, packets_per_second, period_bytes, bytes_per_second, bits_per_second / 1000000);
		sleep(10);
	}

	return NULL;
}

void error(const char *msg)
{
	perror(msg);
	exit(1);
}

int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	char buf[BUFLEN];
	int slen=sizeof(cli_addr);

	memset((char *) &serv_addr, 0, sizeof(serv_addr));

	int n;
	if (argc < 2) {
		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
	}
	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockfd < 0)
		error("ERROR opening socket");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
			sizeof(serv_addr)) < 0)
		error("ERROR on binding");
	int i;

	pthread_t monitoring_thread;
	if (pthread_create(&monitoring_thread, NULL, monitoring_function, NULL) != 0) {
		printf("failed to create monitoring thread");
		return(1);
	}

	while (1) {
		int received_bytes = recvfrom(sockfd, buf, BUFLEN, 0, (struct sockaddr*)&cli_addr, (socklen_t *)&slen);
		if (received_bytes == -1) {
			error("recvfrom()");
		}

		pthread_mutex_lock(&stat_mutex);
		packets++;
		bytes += received_bytes;
		pthread_mutex_unlock(&stat_mutex);
	}
	close(sockfd);
	return 0;
}
