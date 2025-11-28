
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

static void append_hex_byte(char *out, size_t *idx, size_t max, unsigned char v)
{
    if (*idx + 2 >= max)
        return;
    static const char *hex = "0123456789ABCDEF";
    out[(*idx)++] = hex[(v >> 4) & 0x0F];
    out[(*idx)++] = hex[v & 0x0F];
    out[*idx] = '\0';
}

unsigned char calculate_parity_even(const char *text)
{
    int ones = 0;
    for (int i = 0; text[i]; i++)
    {
        unsigned char ch = (unsigned char)text[i];
        for (int b = 0; b < 8; b++)
        {
            if (ch & (1 << b))
                ones++;
        }
    }
    return (unsigned char)(ones % 2);
}

void calculate_2d_parity(const char *data, char *out, size_t out_size)
{
    unsigned char row_parity[32] = {0};
    unsigned char col_parity[8] = {0};

    size_t len = strlen(data);
    int rows = (int)((len + 7) / 8);
    if (rows > 32)
        rows = 32;

    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < 8; c++)
        {
            size_t idx = r * 8 + c;
            unsigned char ch = (idx < len) ? (unsigned char)data[idx] : 0;
            row_parity[r] ^= ch;
            col_parity[c] ^= ch;
        }
    }

    size_t p = 0;
    out[0] = '\0';

    for (int r = 0; r < rows; r++)
    {
        append_hex_byte(out, &p, out_size, row_parity[r]);
    }
    for (int c = 0; c < 8; c++)
    {
        append_hex_byte(out, &p, out_size, col_parity[c]);
    }
    out[p] = '\0';
}

unsigned short calculate_crc16(const char *data)
{
    unsigned short crc = 0xFFFF;
    while (*data)
    {
        crc ^= (unsigned char)(*data) << 8;
        for (int i = 0; i < 8; i++)
        {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1;
        }
        data++;
    }
    return crc;
}

void calculate_hamming_parity(const char *data, char *out, size_t out_size)
{
    static const char *hex = "0123456789ABCDEF";
    size_t p = 0;
    out[0] = '\0';

    for (int i = 0; data[i]; i++)
    {
        unsigned char b = (unsigned char)data[i];
        int d[9] = {0};
        for (int k = 0; k < 8; k++)
            d[k + 1] = (b >> k) & 1;

        int code[13] = {0};
        code[3] = d[1];
        code[5] = d[2];
        code[6] = d[3];
        code[7] = d[4];
        code[9] = d[5];
        code[10] = d[6];
        code[11] = d[7];
        code[12] = d[8];

        int p1 = (code[3] ^ code[5] ^ code[7] ^ code[9] ^ code[11]) & 1;
        int p2 = (code[3] ^ code[6] ^ code[7] ^ code[10] ^ code[11]) & 1;
        int p4 = (code[5] ^ code[6] ^ code[7] ^ code[12]) & 1;
        int p8 = (code[8] ^ code[9] ^ code[10] ^ code[11] ^ code[12]) & 1;

        unsigned char nibble = (unsigned char)(p1 | (p2 << 1) | (p4 << 2) | (p8 << 3));

        if (p + 1 >= out_size)
            break;
        out[p++] = hex[nibble & 0x0F];
        out[p] = '\0';
    }
}

unsigned short calculate_checksum16(const char *data)
{
    unsigned long sum = 0;
    size_t len = strlen(data);

    for (size_t i = 0; i < len; i += 2)
    {
        unsigned short word = ((unsigned char)data[i]) << 8;
        if (i + 1 < len)
            word |= (unsigned char)data[i + 1];
        sum += word;
        while (sum >> 16)
            sum = (sum & 0xFFFF) + (sum >> 16);
    }

    unsigned short checksum = (unsigned short)~sum;
    return checksum;
}

void generate_control_info(const char *data, const char *method, char *control, size_t control_size)
{
    control[0] = '\0';

    if (strcmp(method, "PARITY") == 0)
    {
        unsigned char p = calculate_parity_even(data);
        snprintf(control, control_size, "%d", p);
    }
    else if (strcmp(method, "2DPARITY") == 0)
    {
        calculate_2d_parity(data, control, control_size);
    }
    else if (strcmp(method, "CRC16") == 0)
    {
        unsigned short crc = calculate_crc16(data);
        snprintf(control, control_size, "%04X", crc);
    }
    else if (strcmp(method, "HAMMING") == 0)
    {
        calculate_hamming_parity(data, control, control_size);
    }
    else if (strcmp(method, "CHECKSUM") == 0)
    {
        unsigned short cs = calculate_checksum16(data);
        snprintf(control, control_size, "%04X", cs);
    }
    else
    {
        snprintf(control, control_size, "NA");
    }
}

int main(void)
{
    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in server;

    char data[1024];
    char method[32];
    char control[2048];
    char packet[4096];
    int choice;

    printf("=== Client 1 - Data Sender ===\n");

    printf("Enter text to send: ");
    if (!fgets(data, sizeof(data), stdin))
    {
        printf("Input error.\n");
        return 1;
    }
    data[strcspn(data, "\n")] = '\0';

    printf("\nSelect control method:\n");
    printf("1) Parity Bit (Even)\n");
    printf("2) 2D Parity\n");
    printf("3) CRC16\n");
    printf("4) Hamming\n");
    printf("5) Internet Checksum\n");
    printf("Choice: ");
    if (scanf("%d", &choice) != 1)
    {
        printf("Invalid input.\n");
        return 1;
    }
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;

    switch (choice)
    {
    case 1:
        strcpy(method, "PARITY");
        break;
    case 2:
        strcpy(method, "2DPARITY");
        break;
    case 3:
        strcpy(method, "CRC16");
        break;
    case 4:
        strcpy(method, "HAMMING");
        break;
    case 5:
        strcpy(method, "CHECKSUM");
        break;
    default:
        printf("Unknown choice.\n");
        return 0;
    }

    generate_control_info(data, method, control, sizeof(control));

    snprintf(packet, sizeof(packet), "%s|%s|%s", data, method, control);
    printf("\n[Client1] Packet:\n%s\n", packet);

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("WSAStartup failed.\n");
        return 1;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        printf("Socket creation failed.\n");
        WSACleanup();
        return 1;
    }

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(8080);

    printf("\nConnecting to server...\n");
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("Connection failed.\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    if (send(sock, packet, (int)strlen(packet), 0) < 0)
    {
        printf("Send failed.\n");
    }
    else
    {
        printf("[Client1] Packet sent to server.\n");
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}