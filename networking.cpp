#include "router.h"


void setup_socket(int &sockfd, struct sockaddr_in &server_addr)
{
    bzero (&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind (sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));

    int broadcastPermission = 1;
    setsockopt (sockfd, SOL_SOCKET, SO_BROADCAST, (void *)&broadcastPermission, sizeof(broadcastPermission));
}

std::vector<dv_record> setup_distace_vector(int n, con_info my_records[])
{
    std::vector<dv_record> initial_vector;
    for (int i = 0; i < n; i++)
    {
        dv_record aux;
        aux.my = true;
        aux.via = -1;
        aux.distance = my_records[i].distance;
        aux.last_contact = -1;
        aux.network_address = (my_records[i].ip>>my_records[i].mask)<<my_records[i].mask;
        aux.broadcast = my_records[i].broadcast;
        aux.ip = my_records[i].ip;
        aux.mask = my_records[i].mask;
        initial_vector.push_back(aux);
    }
    return initial_vector;
}

struct sockaddr_in get_recipient(int32_t target_ip)
{
	struct sockaddr_in recipient;
	bzero (&recipient, sizeof(recipient));
	recipient.sin_family = AF_INET;
    recipient.sin_addr.s_addr = target_ip;
    recipient.sin_port = htons(PORT);

	return recipient;
}

udp get_msg(dv_record record)
{
    udp msg;
    msg.ip=record.ip;
    msg.mask=record.mask;
    msg.distance=htonl(record.distance);
    return msg;
}

void make_unreachable(std::vector<dv_record> dist_vector, uint idx)
{
    for (uint i = 0; i < dist_vector.size(); i++)
        if (i==idx || dist_vector[i].via == dist_vector[idx].ip)
            dist_vector[i].distance = INF;
}

void make_reachable(std::vector<dv_record> dist_vector, uint idx)
{
    for (uint i = 0; i < dist_vector.size(); i++)
        if (i==idx || dist_vector[i].via == dist_vector[idx].ip)
            dist_vector[i].distance = dist_vector[idx].base_distance;
}

void trim_contact(std::vector<dv_record> dist_vector, uint n)
{
    for (uint i = dist_vector.size() - 1; i > 0; i--)
    {
        dist_vector[i].last_contact-=1;
        if (dist_vector[i].last_contact == 0)
        {
            if (i<n)
            {
                dist_vector[i].last_contact = -1;
                make_unreachable(dist_vector, i);                
            }
            else
                dist_vector.erase(dist_vector.begin()+i);
        }
    }
}

void send_my_vector(std::vector<dv_record> dist_vector, int sockfd, uint n)
{
    for (uint i = 0; i < dist_vector.size(); i++)
        for (uint j = 0; j < n; j++)
            if(i != j && dist_vector[i].last_contact != 0)
            {
                struct sockaddr_in recipient = get_recipient(dist_vector[j].broadcast);
                udp msg = get_msg(dist_vector[i]);

                ssize_t bytes_sent = sendto (
                                        sockfd,
                                        &msg,
                                        sizeof(msg),
                                        0,
                                        (struct sockaddr*)&recipient,
                                        sizeof(recipient)
                                    );

                if (bytes_sent == -1)
                    make_unreachable(dist_vector, j);
                else if (dist_vector[j].distance == INF)
                    make_reachable(dist_vector, j);
            }
    
    trim_contact(dist_vector, n);
}

int32_t find_distance(std::vector<dv_record> dist_vector, struct sockaddr_in sender)
{
    int i = 0;
    while (dist_vector[i].my)
    {
        int32_t aux = (sender.sin_addr.s_addr>>dist_vector[i].mask)<<dist_vector[i].mask;
        if (aux == dist_vector[i].ip)
            return 0;
        else if (aux == dist_vector[i].network_address)
            return dist_vector[i].distance;
        i++;
    }
    return 0;
}

void update_vector(std::vector<dv_record> dist_vector, udp msg, struct sockaddr_in sender)
{
    bool found = false;
    int32_t nett_addr = (msg.ip>>msg.mask)<<msg.mask;
    int32_t distance = find_distance(dist_vector, sender);

    for (uint i = 0; i < dist_vector.size(); i++)
        if (dist_vector[i].network_address == nett_addr)
        {
            found = true;
            if (dist_vector[i].distance < (ntohl(msg.distance) + distance))
            {
                dist_vector[i].distance = (ntohl(msg.distance) + distance);

                if (distance == 0)
                    dist_vector[i].via = -1;
                else
                    dist_vector[i].via = msg.ip;              
            }
        }

    if (!found)
    {
        dv_record aux;
        aux.via = msg.ip;
        aux.network_address=nett_addr;
        aux.distance = ntohl(msg.distance) + distance;
        aux.mask = msg.mask;
        dist_vector.push_back(aux);
    }
    
}

void rec_requests(std::vector<dv_record> dist_vector, int sockfd)
{
    struct sockaddr_in sender;
    socklen_t sender_len = sizeof(sender);
    udp msg;
    if(recvfrom (sockfd, (udp*)&msg, 9, 0, (struct sockaddr*)&sender, &sender_len) == -1)
        fprintf(stderr, "recvfrom error: %s\n", strerror(errno));

    update_vector(dist_vector, msg, sender);
}

void print_current_vector(std::vector<dv_record> dist_vector)
{
    for (uint i = 0; i < dist_vector.size(); i++)
    {
        char ip[17];
        dist_vector[i].network_address = htonl(dist_vector[i].network_address);
        inet_ntop(AF_INET, &dist_vector[i].network_address, ip, sizeof(ip));
        printf("%s/%d ", ip, dist_vector[i].mask);
        if (dist_vector[i].distance == INF)
            printf("unreachable ");
        else
            printf("distance %d ", dist_vector[i].distance);

        if (dist_vector[i].via == -1)
            printf("connected directly\n");
        else
        {
            char via[16];
            dist_vector[i].via = htonl(dist_vector[i].via);
            inet_ntop(AF_INET, &dist_vector[i].via, via, sizeof(via));
            printf("via %s\n", via);
        }
    }
    printf("----------------------------------\n");
}

fd_set get_descriptors(int sockfd)
{
	fd_set descriptors;
	FD_ZERO (&descriptors);
	FD_SET (sockfd, &descriptors);
	
	return descriptors;
}


void network_loop(int n, con_info my_records[], int sockfd)
{
    
    std::vector<dv_record> dist_vector = setup_distace_vector(n, my_records);
    fd_set descriptors = get_descriptors(sockfd);
    struct timeval tv; tv.tv_sec = 0; tv.tv_usec = 0;
    while (1)
    {
        int ready = select(sockfd+1, &descriptors, NULL, NULL, &tv);
        if (ready > 0)
            rec_requests(dist_vector, sockfd);
        else
        {
            send_my_vector(dist_vector, sockfd, n);
            tv.tv_sec = TURA;
        }
        print_current_vector(dist_vector);
    }
}
