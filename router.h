#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <cstdint>
#include <cassert>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <inttypes.h>
#include <iostream>
#include <string>
#include <cstdio>
#include <vector>

#define TURA 5
#define PORT 54321
#define FRESH_CONTACT 5
#define LOST_COTACT 3
#define INF 4294967295

typedef struct
{
    int32_t ip;
    int8_t mask;
    uint32_t distance;
    int32_t broadcast;
} con_info;

typedef struct
{
    int32_t network_address;
    int32_t broadcast;
    int32_t ip;
    int32_t via;
    uint32_t distance;
    int32_t base_distance;
    int8_t mask;
    int last_contact;
    bool my;
} dv_record;

typedef struct
{
    int32_t ip;
    int8_t mask;
    uint32_t distance;
} udp;

void parse_input(int &n, con_info my_records[]);
void add_broadcast_address(int n, con_info my_records[]);
void setup_socket(int &sockfd, struct sockaddr_in &server_addr);
void network_loop(int n, con_info my_records[], int sockfd);
