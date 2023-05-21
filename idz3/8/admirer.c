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

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation error");
        return 10;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr.sin_port = htons(port);

    int max_retries = 10;
    int retry_interval = 1;
    int attempt = 0;

    while (attempt <= max_retries) {
        if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            close(sock);
            if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                perror("Socket creation error");
                return 10;
            }
            
            printf("Can't connect to server. Retry: attempt %d of %d\n", attempt, max_retries);
            ++attempt;
            sleep(retry_interval);

            if (attempt == max_retries) {
                perror("Connection error");
                return 10;
            }
        } else {
            break;
        }
    }

    char valentine[BUFFER_SIZE];
    sprintf(valentine, "Unique valentine from %d. %s", getpid(), message_template);
    valentine[strlen(valentine)] = '\0';

    send(sock, valentine, strlen(valentine) + 1, 0);

    printf("Admirer is waiting for answer\n");

    char buffer[BUFFER_SIZE];
    ssize_t recv_result;

    while (1) {
        recv_result = recv(sock, buffer, sizeof(buffer), 0);
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