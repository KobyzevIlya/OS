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

    struct sockaddr_in address;
    int addrlen = sizeof(address);

    struct sockaddr_in client_address[n];
    struct sockaddr_in father_address;

    if ((server_fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == 0) {
        perror("Socket creation error");
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

    int count = 0;
    int it = 0;
    char buffer[BUFFER_SIZE];
    char report[512];

    unsigned int father_addrlen = sizeof(father_addrlen);
    ssize_t recv_result = recvfrom(server_fd, buffer, sizeof(buffer), 0, (struct sockaddr *) &father_address, &father_addrlen);

    while (count < n) {
        if (it >= 10) it = 0;

        unsigned int client_addrlen = sizeof(client_address[it]);
        ssize_t recv_result = recvfrom(server_fd, buffer, sizeof(buffer), 0, (struct sockaddr *) &client_address[it], &client_addrlen);
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
            sendto(server_fd, report, strlen(report) + 1, 0, (struct sockaddr *) &father_address, sizeof(father_address));
        }
    }

    int chosen = rand() % n;
    printf("Beauty carefully considered everything and chose the number %d\n", chosen + 1);

    memset(report, '\0', sizeof(report));
    sprintf(report, "Beauty carefully considered everything and chose the number %d", chosen + 1);
    sendto(server_fd, report, strlen(report) + 1, 0, (struct sockaddr *) &father_address, sizeof(father_address));

    char yes[10] = "YES";
    yes[4] = '\0';
    char no[10] = "NO";
    no[3] = '\0';
    for (int i = 0; i < n; ++i) {
        if (i == chosen) {
            sendto(server_fd, yes, strlen(yes) + 1, 0, (struct sockaddr *) &client_address[i], sizeof(client_address[i]));

            memset(report, '\0', sizeof(report));
            sprintf(report, "Beauty sent YES to admirer №%d", i + 1);
            sendto(server_fd, report, strlen(report) + 1, 0, (struct sockaddr *) &father_address, sizeof(father_address));
        } else {
            sendto(server_fd, no, strlen(no) + 1, 0, (struct sockaddr *) &client_address[i], sizeof(client_address[i]));

            memset(report, '\0', sizeof(report));
            sprintf(report, "Beauty sent YES to admirer №%d", i + 1);
            sendto(server_fd, report, strlen(report) + 1, 0, (struct sockaddr *) &father_address, sizeof(father_address));
        }
    }

    memset(report, '\0', sizeof(report));
    sprintf(report, "Done");
    sendto(server_fd, report, strlen(report) + 1, 0, (struct sockaddr *) &father_address, sizeof(father_address));

    close(server_fd);

    return 0;
}