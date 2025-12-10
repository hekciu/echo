#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <pthread.h>


void* connection_handler(void *);


int main(void) {
    int socket_desc;
    struct sockaddr_in server, client;

    if((socket_desc = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        puts("could not create socket");
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = 8888;

    if (bind(socket_desc, (struct sockaddr*)&server, sizeof(server)) < 0) {
        puts("bind failed");
        return 1;
    }

    if (listen(socket_desc, 1) < 0) {
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

        pthread_t sniffer_thread;

        int* new_socket_ptr = malloc(1);
        *new_socket_ptr = new_socket;

        if (pthread_create(&sniffer_thread, NULL, connection_handler, (void*)new_socket_ptr) < 0) {
            puts("could not create thread");
        }
    }

    if (new_socket < 0) {
        perror("accept failed");
    }

    return 0;
}


void* connection_handler(void* socket_desc) {
    int sock = *(int*)socket_desc;

    const char* msg = "hello from separate unix process!\n";

    write(sock, msg, strlen(msg));

    ssize_t read_size;
    char client_message[2000];

    while((read_size = recv(sock, client_message, 2000, 0)) > 0) {
        printf("writing:\n\n%s\nto client\n", client_message);

        write(sock, client_message, strlen(client_message));
    }

    if(read_size == 0) {
        puts("client disconnected");
        fflush(stdout);
    } else if(read_size == -1) {
        perror("recv failed");
    }

    free(socket_desc);

    return 0;
}
