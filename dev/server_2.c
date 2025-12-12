#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>


#define stdout 1


void connection_handler(int sock);


int my_fork() {
    return syscall(57);
}


int my_socket(int family, int type, int protocol) {
    return syscall(41, family, type, protocol);
}


int my_bind(int fd, struct sockaddr* umyaddr, int addrlen) {
    return syscall(49, fd, umyaddr, addrlen);
}


int my_listen(int fd, int backlog) {
    return syscall(50, fd, backlog);
}


int my_setsockopt(int fd, int level, int optname, void* optval, int optlen) {
    syscall(54, fd, level, optname, optval, optlen);
}


int my_write(unsigned int fd, const char* buf, size_t count) {
    syscall(1, fd, buf, count);
}


void my_puts(const char* msg) {
    my_write(stdout, msg, strlen(msg));
    my_write(stdout, "\n", 1);
}


int main(void) {
    int socket_desc;
    struct sockaddr_in server, client;

    if((socket_desc = my_socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        my_puts("could not create socket");
    }

    int reuse = 1;
    my_setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(2137);

    if (my_bind(socket_desc, (struct sockaddr*)&server, sizeof(server)) < 0) {
        my_puts("bind failed");
        return 1;
    }

    if (my_listen(socket_desc, 3) < 0) {
        my_puts("listen failed");
        return 1;
    }

    my_puts("waiting from incoming connections");

    int c = sizeof(struct sockaddr_in);
    int new_socket;

    while((new_socket = accept(socket_desc, (struct sockaddr*)&client, (socklen_t*)&c))) {
        char* client_ip = inet_ntoa(client.sin_addr);
        int client_port = ntohs(client.sin_port);

        my_puts("client connected");

        const char* msg = "connected, starting separate thread handler...\n";

        my_write(new_socket, msg, strlen(msg));

        int pid = my_fork();

        if (pid < 0) {
            my_puts("fork failed");
            return 1;
        } else if (pid == 0) {
            connection_handler(new_socket);
        }
    }

    if (new_socket < 0) {
        my_puts("accept failed");
    }

    return 0;
}


void connection_handler(int sock) {
    const char* msg = "hello from separate unix process!\n";

    my_write(sock, msg, strlen(msg));

    ssize_t read_size;

    char client_message[2000];

    while((read_size = recv(sock, client_message, 2000, 0)) > 0) {
        my_write(sock, client_message, strlen(client_message));

        memset(client_message, 0, 2000);
    }

    if(read_size == 0) {
        my_puts("client disconnected");
        // fflush(stdout);
    } else if(read_size == -1) {
        my_puts("recv failed");
    }
}
