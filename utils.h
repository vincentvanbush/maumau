#ifndef UTILS_H
#define UTILS_H

#include <netinet/in.h>

int create_socket();

struct sockaddr_in local_address(int);

void bind_socket(int, struct sockaddr_in);

void close_socket(int);

#endif