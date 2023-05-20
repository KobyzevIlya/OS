#include "common.h"

int server_fd;

void close_sock() {
    close(server_fd);
}

void handle_sigint(int sig) {
    close_sock();
    exit(0);
}

void father_block(int father) {
    char father_check[10];
    ssize_t father_recv_result;
    father_recv_result = 0;
    while (father_recv_result == 0) {
        father_recv_result = recv(father, father_check, sizeof(father_check), 0);
        if (father_recv_result == -1) {
            perror("Receive error");
            exit(10);
        }
    }
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
    int father;
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
    if ((father = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
        perror("Accept error");
        close(server_fd);
        return 10;
    }
    char message[10];
    for (int i = 0; i < n; ++i) {
        ssize_t recv_result = recv(sockets[i], message, CHECK_MESSAGE_SIZE, 0);
        if (recv_result <= 0) {
            perror("Receive error");
            return 10;
        } else {
            if (strcmp(message, "admirer") != 0) {
                int tmp = sockets[i];
                sockets[i] = father;
                father = tmp;
            } 
        }
    }

    char report[512];
    memset(report, '\0', sizeof(report));
    sprintf(report, "Beauty is ready to accept valentines");
    send(father, report, strlen(report) + 1, 0);
    father_block(father);
    
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

            memset(report, '\0', sizeof(report));
            sprintf(report, "Beauty received valentine №%d. Message: %s", it, buffer);
            send(father, report, strlen(report) + 1, 0);
            father_block(father);
        }
    }

    int chosen = rand() % n;
    printf("Beauty carefully considered everything and chose the number %d\n", chosen + 1);

    memset(report, '\0', sizeof(report));
    sprintf(report, "Beauty carefully considered everything and chose the number %d", chosen + 1);
    send(father, report, strlen(report) + 1, 0);
    father_block(father);

    char yes[10] = "YES";
    yes[4] = '\0';
    char no[10] = "NO";
    no[3] = '\0';
    for (int i = 0; i < n; ++i) {
        if (i == chosen) {
            send(sockets[i], yes, strlen(yes) + 1, 0);
    
            memset(report, '\0', sizeof(report));
            sprintf(report, "Beauty sent YES to admirer №%d", i + 1);
            send(father, report, strlen(report) + 1, 0);
            father_block(father);
        } else {
            send(sockets[i], no, strlen(no) + 1, 0);

            memset(report, '\0', sizeof(report));
            sprintf(report, "Beauty sent NO to admirer №%d", i + 1);
            send(father, report, strlen(report) + 1, 0);
            father_block(father);
        }
    }

    memset(report, '\0', sizeof(report));
    sprintf(report, "Done");
    send(father, report, strlen(report) + 1, 0);

    while (1) {
        ssize_t recv_result = recv(father, buffer, sizeof(buffer), 0);
        if (recv_result == -1) {
            perror("Receive error");
            return 10;
        } else if (recv_result == 0) {
            continue;
        } else {
            break;
        }
    }

    close(server_fd);
    close(father);
    for (int i = 0; i < n; ++i) {
        close(sockets[i]);
    }

    return 0;
}