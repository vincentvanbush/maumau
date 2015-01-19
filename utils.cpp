#include "utils.h"
#include "messages.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <json/json.h>
#include <json/writer.h>
#include <json/reader.h>
#include <string>
#include <algorithm>

int create_socket() {
	// Initialize UDP socket
	int socket_fd;
	if ((socket_fd = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror ("Cannot open main socket");
		exit (EXIT_FAILURE);
	}
	else {
		fprintf(stderr, "Created socket %d\n", socket_fd);
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
	int optval = 1;
	setsockopt (socket_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
	if (bind (socket_fd, (struct sockaddr*) &socket_addr, sizeof socket_addr) < 0) {
		perror ("Cannot bind socket");
    	exit (EXIT_FAILURE);
	}
	else fprintf(stderr, "Bound socket %d to server\n", socket_fd);
}

void close_socket(int socket_fd) {
	// Close socket
	if (close (socket_fd) < 0) {
		perror("Error closing socket");
		exit (EXIT_FAILURE);
	}
	else fprintf(stderr, "Closed socket %d\n", socket_fd);
}

Json::Value &recv_message(int rcv_sck, int len) {
	char *msg_buf = new char[len];
	int bytes_read = 0;
	int x;
	while (bytes_read < len) {
		if ((x = recv (rcv_sck, msg_buf + bytes_read, len - bytes_read, 0)) < 0) {
			perror ("Error receiving data");
			exit (EXIT_FAILURE);
		}
		else {
			bytes_read += x;
			printf ("Read %d bytes\n", x);
		}
	}
	printf ("Total: %d bytes\n", bytes_read);
	std::string msg_str(msg_buf);
	Json::Reader reader;
	Json::Value *ret = new Json::Value();
	if (!reader.parse(msg_str, *ret)) {
			printf ("Error parsing: %s\n", msg_str.c_str());
	}
	printf ("Content: %s\n", msg_str.c_str());
	return *ret;
}

void send_message(int socket_fd, Json::Value &msg) {
	Json::FastWriter writer;
	std::string msg_str = writer.write(msg);
	const char* msg_c = msg_str.c_str();

	int c_size = strlen (msg_c);
	printf ("Total message size: %d bytes\n", c_size);
	if (send (socket_fd, &c_size, sizeof (int), 0) < 0) {
		perror ("Error sending msg length to client socket");
		exit (EXIT_FAILURE);
	}
	else {
		fprintf(stderr, "Sent msg length to client\n");
	}

	int sent_bytes = 0;
	while (sent_bytes < c_size) {
		int x;
		if ((x = send (socket_fd, msg_c + sent_bytes, c_size - sent_bytes, 0)) < 0) {
			perror ("Error sending message to client socket");
			exit (EXIT_FAILURE);
		}
		else {
			sent_bytes += x;
			fprintf(stderr, "Sent %d bytes to client, total: %d\n", x, sent_bytes);
		}
	}

	printf ("Content: %s\n", msg_c);
}
