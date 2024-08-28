#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define BUFLEN 65536
pthread_mutex_t stat_mutex = PTHREAD_MUTEX_INITIALIZER;

long bytes = 0;
long packets = 0;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

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

		fprintf(stderr, "packets=%d, packets/second=%f, bytes=%d, bytes/second=%f, Mbits/second=%f\n",
				period_packets, packets_per_second, period_bytes, bytes_per_second, bits_per_second / 1000000);
		sleep(10);
	}

	return NULL;
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[BUFLEN];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }

	pthread_t monitoring_thread;
	if (pthread_create(&monitoring_thread, NULL, monitoring_function, NULL) != 0) {
		printf("failed to create monitoring thread");
		return(1);
	}


    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    int server_len = sizeof(serv_addr);
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");
    bzero(buffer,BUFLEN);

	while (1) {
		int received_bytes = read(sockfd, buffer, BUFLEN);
		if (received_bytes == -1) {
			printf("Error on read");
		}

		write(1, buffer, received_bytes);
		pthread_mutex_lock(&stat_mutex);
		packets++;
		bytes += received_bytes;
		pthread_mutex_unlock(&stat_mutex);
	}

    close(sockfd);
    return 0;
}
