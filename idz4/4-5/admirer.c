#include "common.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <server ip> <port>\n", argv[0]);
        return 10;
    }

    printf("Your pid: %d\n", getpid());

    char message_template[] = "Written with love after much thought. Template written using freevalentines.com, remove this sentence before sending.";

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

    char valentine[BUFFER_SIZE];
    sprintf(valentine, "Unique valentine from %d. %s", getpid(), message_template);
    valentine[strlen(valentine)] = '\0';

    sendto(sock, valentine, strlen(valentine) + 1, 0, (struct sockaddr *) &server_addr, sizeof(server_addr));

    printf("Admirer is waiting for answer\n");

    char buffer[BUFFER_SIZE];
    ssize_t recv_result;

    while (1) {
        unsigned int server_addrlen = sizeof(server_addr);
        recv_result = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *) &server_addr, &server_addrlen);
        if (recv_result < 0) {
            perror("Receive error");
            close(sock);
            return 10;
        } else if (recv_result == 0) {
            continue;
        } else {
            break;
        }
    }

    if (strcmp(buffer, "YES")) {
        printf("There are more beauties around\n");
    } else if (strcmp(buffer, "NO")) {
        printf("I'm very happy\n");
    } else {
        printf("other\n");
    }

    close(sock);

    return 0;
}