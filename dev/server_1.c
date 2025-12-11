#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>


void connection_handler(int sock);


int main(void) {
    int socket_desc;
    struct sockaddr_in server, client;

    if((socket_desc = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        puts("could not create socket");
    }

    int reuse = 1;
    setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(2137);

    if (bind(socket_desc, (struct sockaddr*)&server, sizeof(server)) < 0) {
        puts("bind failed");
        return 1;
    }

    if (listen(socket_desc, 3) < 0) {
        puts("listen failed");
        return 1;
    }

    puts("waiting from incoming connections");

    int c = sizeof(struct sockaddr_in);
    int new_socket;

    while((new_socket = accept(socket_desc, (struct sockaddr*)&client, (socklen_t*)&c))) {
        char* client_ip = inet_ntoa(client.sin_addr);
        int client_port = ntohs(client.sin_port);

        printf("client connected from %s:%d\n", client_ip, client_port);

        const char* msg = "connected, starting separate thread handler...\n";

        write(new_socket, msg, strlen(msg));

        int pid = fork();

        if (pid < 0) {
            perror("fork failed");
            return 1;
        } else if (pid == 0) {
            connection_handler(new_socket);
        }
    }

    if (new_socket < 0) {
        perror("accept failed");
    }

    return 0;
}


void connection_handler(int sock) {
    const char* msg = "hello from separate unix process!\n";

    write(sock, msg, strlen(msg));

    ssize_t read_size;

    char client_message[2000];

    while((read_size = recv(sock, client_message, 2000, 0)) > 0) {
        printf("writing:\n\n%s\nto client\n", client_message);

        write(sock, client_message, strlen(client_message));

        memset(client_message, 0, 2000);
    }

    if(read_size == 0) {
        puts("client disconnected");
        fflush(stdout);
    } else if(read_size == -1) {
        perror("recv failed");
    }
}
