#ifndef UTILS_H
#define UTILS_H

#include <netinet/in.h>
#include <json/json.h>
#include <string>

int create_socket();

struct sockaddr_in local_address(int);

void bind_socket(int, struct sockaddr_in);

void close_socket(int);

void send_message(int, Json::Value &);

Json::Value &recv_message(int, int);

#endif
