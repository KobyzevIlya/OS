#include "common.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <server ip> <port>\n", argv[0]);
        return 10;
    }

    printf("Your pid: %d\n", getpid());

    char *server_ip = argv[1];
    int port = atoi(argv[2]);

    int sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        perror("Socket creation error");
        return 10;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr.sin_port = htons(port);

    sendto(sock, "something", sizeof("something"), 0, (struct sockaddr *) &server_addr, sizeof(server_addr));

    char buffer[BUFFER_SIZE];
    while(1) {
        unsigned int addrlen = sizeof(server_addr);
        ssize_t recv_result = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *) &server_addr, &addrlen);
        if (recv_result == -1) {
            perror("Receive error father");
            return 10;
        } else if (recv_result == 0) {
            continue;
        } else {
            if (strcmp(buffer, "Done") == 0) {
                printf("Beauty goes to sleep\n");
                break;
            }
            send(sock, "1", sizeof("1"), 0);

            printf("Report: %s\n", buffer);
        }
    }

    sendto(sock, "Done", sizeof("Done"), 0, (struct sockaddr *) &server_addr, sizeof(server_addr));

    printf("Done\n");

    close(sock);

    return 0;
}