#include "utils.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>


int create_socket() {
	// Initialize UDP socket
	int socket_fd;
	if ((socket_fd = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		perror ("Cannot open main socket");
		exit (EXIT_FAILURE);
	}
	else {
		printf("Created socket %d\n", socket_fd);
		return socket_fd;
	}
}

struct sockaddr_in local_address(int port) {
	struct sockaddr_in addr;
	bzero(&addr, sizeof addr);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons (port);
	return addr;
}

void bind_socket(int socket_fd, struct sockaddr_in socket_addr) {
	// Bind UDP socket to server address
	if (bind (socket_fd, (struct sockaddr*) &socket_addr, sizeof socket_addr) < 0) {
		perror ("Cannot bind socket");
    	exit (EXIT_FAILURE);
	}
	else printf("Bound socket %d to server\n", socket_fd);
}

void close_socket(int socket_fd) {
	// Close socket
	if (close (socket_fd) < 0) {
		perror("Error closing socket");
		exit (EXIT_FAILURE);
	}
	else printf("Closed socket %d\n", socket_fd);
}
