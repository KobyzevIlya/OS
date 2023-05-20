#include "common.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <server ip> <port>\n", argv[0]);
        return 10;
    }

    printf("Your pid: %d\n", getpid());

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
            char message[CHECK_MESSAGE_SIZE] = "father";
            message[6] = '\0';
            send(sock, message, CHECK_MESSAGE_SIZE, 0);

            break;
        }
    }

    char buffer[BUFFER_SIZE];
    while(1) {
        ssize_t recv_result = recv(sock, buffer, sizeof(buffer), 0);
        if (recv_result == -1) {
            perror("Receive error");
            return 10;
        } else if (recv_result == 0) {
            continue;
        } else {
            if (strcmp(buffer, "Done") == 0) {
                printf("Beauty goes to sleep\n");
                break;
            }

            printf("Report: %s\n", buffer);
        }
    }

    send(sock, "Done", sizeof("Done"), 0);

    printf("Done\n");

    close(sock);

    return 0;
}