#include "common.h"

int server_fd;

int fathers[100];
int is_active[100];
int active_count = 0;
int fathers_iterator = 0;

void close_sock() {
    close(server_fd);
}

void handle_sigint(int sig) {
    close_sock();
    exit(0);
}

// чтобы send() не блокировала процесс при отправке в закрытый канал
void sigpipe_handler(int signum) {
    
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

void no_fathers() {
    for (int i = 0; i < 100; ++i) {
        fathers[i] = 0;
        is_active[i] = 0;
    }
}

void add_father(int fd) {
    if (active_count >= 100) {
        printf("Max active watchers\n");
        return;
    }

    if (fathers_iterator >= 100) {
        fathers_iterator = 0;
    }
    while (is_active[fathers_iterator]) {
        if (fathers_iterator == 99) {
            fathers_iterator = 0;
        }
        ++fathers_iterator;
    }
    fathers[fathers_iterator] = fd;
    is_active[fathers_iterator] = 1;
    ++active_count;
}

void send_reports(char report[512]) {
    for (int i = 0; i < 100; ++i) {
        int status = send(fathers[i], report, strlen(report) + 1, 0);
        if (status == -1 && is_active[i]) {
            --active_count;
            is_active[i] = 0;
        }
    }
}

void get_connections_from_queue(struct sockaddr_in address, int addrlen) {
    for (int i = 0; i < 100; ++i) {
        int connection = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (connection != -1) {
            add_father(connection);
        }
    }
}

void ten_secs() {
    printf("Time for new watchers\n");
    for (int i = 0; i < 10; ++i) {
        printf("%d\n", 10-i);
        sleep(1);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <port> <num of admirers>\n", argv[0]);
        return 10;
    }

    srand(time(NULL));
    signal(SIGINT, handle_sigint);
    signal(SIGPIPE, sigpipe_handler);

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

    int flags = fcntl(server_fd, F_GETFL, 0);
    fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);

    ten_secs();

    no_fathers();  

    char report[512];
    memset(report, '\0', sizeof(report));
    sprintf(report, "Beauty is ready to accept valentines");
    get_connections_from_queue(address, addrlen);  
    send_reports(report);
    
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
            get_connections_from_queue(address, addrlen);
            send_reports(report); 
        }
    }

    int chosen = rand() % n;
    printf("Beauty carefully considered everything and chose the number %d\n", chosen + 1);

    ten_secs();

    memset(report, '\0', sizeof(report));
    sprintf(report, "Beauty carefully considered everything and chose the number %d", chosen + 1);
    get_connections_from_queue(address, addrlen);
    send_reports(report);

    char yes[10];
    char no[10];
    memset(yes, '\0', sizeof(yes));
    memset(no, '\0', sizeof(no));
    sprintf(yes, "YES");
    sprintf(no, "NO");

    for (int i = 0; i < n; ++i) {
        if (i == chosen) {
            send(sockets[i], yes, strlen(yes) + 1, 0);
    
            memset(report, '\0', sizeof(report));
            sprintf(report, "Beauty sent YES to admirer №%d", i + 1);
            get_connections_from_queue(address, addrlen);
            send_reports(report);
        } else {
            send(sockets[i], no, strlen(no) + 1, 0);

            memset(report, '\0', sizeof(report));
            sprintf(report, "Beauty sent NO to admirer №%d", i + 1);
            get_connections_from_queue(address, addrlen);
            send_reports(report);
        }
    }

    memset(report, '\0', sizeof(report));
    sprintf(report, "Done");
    get_connections_from_queue(address, addrlen);
    send_reports(report);

    close(server_fd);
    for (int i = 0; i < n; ++i) {
        close(sockets[i]);
    }
    for (int i = 0; i < 100; ++i) {
        close(fathers[i]);
    }

    return 0;
}