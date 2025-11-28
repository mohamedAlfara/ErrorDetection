

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

void bit_flip(char *data)
{
    int len = (int)strlen(data);
    if (len == 0)
        return;
    int idx = rand() % len;
    int bit = rand() % 8;
    data[idx] ^= (1 << bit);
}

void substitute_char(char *data)
{
    int len = (int)strlen(data);
    if (len == 0)
        return;
    int idx = rand() % len;
    data[idx] = 'A' + (rand() % 26);
}

void delete_char(char *data)
{
    int len = (int)strlen(data);
    if (len <= 1)
        return;
    int idx = rand() % len;
    memmove(&data[idx], &data[idx + 1], len - idx);
}

void insert_char(char *data)
{
    int len = (int)strlen(data);
    if (len >= 1020)
        return;
    int idx = rand() % (len + 1);

    for (int i = len; i >= idx; i--)
    {
        data[i + 1] = data[i];
    }
    data[idx] = 'a' + (rand() % 26);
}

void swap_chars(char *data)
{
    int len = (int)strlen(data);
    if (len <= 1)
        return;
    int idx = rand() % (len - 1);
    char temp = data[idx];
    data[idx] = data[idx + 1];
    data[idx + 1] = temp;
}

void multiple_bit_flips(char *data)
{
    int len = (int)strlen(data);
    if (len == 0)
        return;
    int flips = 1 + rand() % 5;
    for (int i = 0; i < flips; i++)
    {
        bit_flip(data);
    }
}

void burst_error(char *data)
{
    int len = (int)strlen(data);
    if (len < 3)
        return;
    int burst_len = 3 + rand() % 6;
    if (burst_len > len)
        burst_len = len;
    int start = rand() % (len - burst_len + 1);
    for (int i = start; i < start + burst_len; i++)
    {
        data[i] = 'A' + (rand() % 26);
    }
}

void apply_random_error(char *data)
{
    int choice = 1 + rand() % 7;
    printf("\n[Server] Applying error type: %d\n", choice);
    switch (choice)
    {
    case 1:
        bit_flip(data);
        break;
    case 2:
        substitute_char(data);
        break;
    case 3:
        delete_char(data);
        break;
    case 4:
        insert_char(data);
        break;
    case 5:
        swap_chars(data);
        break;
    case 6:
        multiple_bit_flips(data);
        break;
    case 7:
        burst_error(data);
        break;
    default:
        break;
    }
}

int main(void)
{
    srand((unsigned int)time(NULL));

    WSADATA wsa;
    SOCKET server_sock, c1_sock, c2_sock;
    struct sockaddr_in server, client1, client2;
    int c1_len, c2_len;
    char buffer[4096];

    printf("=== Server ===\n");

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("WSAStartup failed.\n");
        return 1;
    }

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == INVALID_SOCKET)
    {
        printf("Socket creation failed.\n");
        WSACleanup();
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8080);

    if (bind(server_sock, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
    {
        printf("Bind failed.\n");
        closesocket(server_sock);
        WSACleanup();
        return 1;
    }

    if (listen(server_sock, 2) < 0)
    {
        printf("Listen failed.\n");
        closesocket(server_sock);
        WSACleanup();
        return 1;
    }

    printf("Waiting for Client1...\n");
    c1_len = sizeof(client1);
    c1_sock = accept(server_sock, (struct sockaddr *)&client1, &c1_len);
    if (c1_sock == INVALID_SOCKET)
    {
        printf("Accept Client1 failed.\n");
        closesocket(server_sock);
        WSACleanup();
        return 1;
    }
    printf("Client1 connected.\n");

    int recv_size = recv(c1_sock, buffer, sizeof(buffer) - 1, 0);
    if (recv_size <= 0)
    {
        printf("Receive from Client1 failed.\n");
        closesocket(c1_sock);
        closesocket(server_sock);
        WSACleanup();
        return 1;
    }
    buffer[recv_size] = '\0';

    printf("\n[Server] Packet from Client1:\n%s\n", buffer);

    char data[2048], method[64], control[2048];
    data[0] = method[0] = control[0] = '\0';

    if (sscanf(buffer, "%[^|]|%[^|]|%s", data, method, control) != 3)
    {
        printf("Packet format error.\n");
        closesocket(c1_sock);
        closesocket(server_sock);
        WSACleanup();
        return 1;
    }

    printf("\nOriginal DATA  : %s\n", data);
    printf("Method         : %s\n", method);
    printf("Control (sent) : %s\n", control);

    apply_random_error(data);

    printf("\nCorrupted DATA : %s\n", data);

    char new_packet[4096];
    snprintf(new_packet, sizeof(new_packet), "%s|%s|%s", data, method, control);

    printf("\n[Server] New Packet to send to Client2:\n%s\n", new_packet);

    closesocket(c1_sock);

    printf("\nWaiting for Client2...\n");
    c2_len = sizeof(client2);
    c2_sock = accept(server_sock, (struct sockaddr *)&client2, &c2_len);
    if (c2_sock == INVALID_SOCKET)
    {
        printf("Accept Client2 failed.\n");
        closesocket(server_sock);
        WSACleanup();
        return 1;
    }
    printf("Client2 connected.\n");

    if (send(c2_sock, new_packet, (int)strlen(new_packet), 0) < 0)
    {
        printf("Send to Client2 failed.\n");
    }
    else
    {
        printf("[Server] Packet sent to Client2.\n");
    }

    closesocket(c2_sock);
    closesocket(server_sock);
    WSACleanup();
    return 0;
}