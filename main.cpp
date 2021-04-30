#include "router.h"

int main()
{
    int n;
    con_info my_records[255];
    int sockfd;
    struct sockaddr_in server_addr;
    
    parse_input(n, my_records);
    add_broadcast_address(n, my_records);

    setup_socket(sockfd, server_addr);
    network_loop(n, my_records, sockfd);
    
    return 0;
}
