#include "router.h"

void parse_input(int &n, con_info my_records[])
{
    scanf("%d", &n);
    for (int i = 0; i < n; i++)
    {
        int32_t ip_1, ip_2, ip_3, ip_4;
        scanf("%d.%d.%d.%d/%" SCNd8 " distance %d", &ip_1, &ip_2, &ip_3, &ip_4, &my_records[i].mask, &my_records[i].distance);
        my_records[i].ip = (ip_1<<24) + (ip_2<<16) + (ip_3<<8) + ip_4; //Wiem o funkcji inet_pton, ale nie działała gdy używałem tablic charów, a przez pobieranie z wejścia nie mogłem używać const charów
    }
}

void add_broadcast_address(int n, con_info my_records[])
{
    for (int i = 0; i < n; i++)
        my_records[i].broadcast = my_records[i].ip|( (1<<my_records[i].mask) - 1);
}