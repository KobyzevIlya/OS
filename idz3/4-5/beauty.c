#include "common.h"

int server_fd;

void close_sock() {
    close(server_fd);
}

void handle_sigint(int sig) {
    close_sock();
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <port> <num of admirers>\n", argv[0]);
        return 10;
    }

    srand(time(NULL));
    signal(SIGINT, handle_sigint);

    int n = atoi(argv[2]);
    int port = atoi(argv[1]);

    int sockets[n];
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation error");
        return 10;
    }

    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("Setsockopt error");
        close(server_fd);
        return 10;
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);

    printf("Server IP: %s\n", inet_ntoa(address.sin_addr));

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind error");
        close(server_fd);
        return 10;
    }

    if (listen(server_fd, n) < 0) {
        perror("Listen error");
        close(server_fd);
        return 10;
    }

    for (int i = 0; i < n; ++i) {
        if ((sockets[i] = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept error");
            close(server_fd);
            return 10;
        }
    }

    int count = 0;
    int it = 0;
    char buffer[BUFFER_SIZE];

    while (count < n) {
        if (it >= 10) it = 0;

        ssize_t recv_result = recv(sockets[it], buffer, sizeof(buffer), 0);
        if (recv_result == -1) {
            perror("Receive error");
            return 10;
        } else if (recv_result == 0) {
            ++it;
            continue;
        } else {
            ++it;
            ++count;
            printf("Received message from admirer №%d.\nMessage: %s\n\n", it, buffer);
        }
    }

    int chosen = rand() % n;
    printf("Beauty carefully considered everything and chose the number %d\n", chosen + 1);

    char yes[10] = "YES";
    yes[4] = '\0';
    char no[10] = "NO";
    no[3] = '\0';
    for (int i = 0; i < n; ++i) {
        if (i == chosen) {
            send(sockets[i], yes, strlen(yes) + 1, 0);
        } else {
            send(sockets[i], no, strlen(no) + 1, 0);
        }
    }

    close(server_fd);
    for (int i = 0; i < n; ++i) {
        close(sockets[i]);
    }

    return 0;
}