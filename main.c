/* Basic defines for cleaner code */

#define stdout 1
#define NULL 0


/* Declaration of connection handler for separate process */

void connection_handler(int sock);


/*
    Assembly-defined syscall's
*/

/* Generic syscall with at most 5 arguments */
void* syscall5(
        void* number,
        void* arg1,
        void* arg2,
        void* arg3,
        void* arg4,
        void* arg5
);

/* One hardcoded syscall that needs 6 arguments */
void* sys_recvfrom(
        void* arg1,
        void* arg2,
        void* arg3,
        void* arg4,
        void* arg5,
        void* arg6
);


/* Typedef's, theit size were obtained from calls to sizeof(), definition may vary between machines */
typedef long unsigned int size_t;
typedef long ssize_t;

typedef unsigned long int in_addr_t;
typedef unsigned short in_port_t;

typedef unsigned int socklen_t;
typedef unsigned short sa_family_t;


/* Socket-specific defines (can be found inside libc's include files)*/
#define AF_INET 2
#define SOCK_STREAM 1
#define SOL_SOCKET 1
#define SO_REUSEADDR 2
#define INADDR_ANY ((in_addr_t) 0x00000000)


/* Socket-specific structs (can be found in manpages) */
struct sockaddr {
   sa_family_t     sa_family;      /* Address family */
   char            sa_data[];      /* Socket address */
};

struct sockaddr_storage {
   sa_family_t     ss_family;      /* Address family */
};

struct in_addr {
   in_addr_t s_addr;
};

struct sockaddr_in {
   sa_family_t     sin_family;     /* AF_INET */
   in_port_t       sin_port;       /* Port number */
   struct in_addr  sin_addr;       /* IPv4 address */
};


/* Generic libc helper functions */
size_t strlen(const char* s) {
    size_t output = 0;

    while(*s++ != '\0') output++;

    return output;
}

void* memset(void* dest, int data, size_t count) {
    unsigned char value = (unsigned char)data;

    for (size_t i = 0; i < count; i++) {
        *((unsigned char*)dest + i) = value; 
    }

    return dest;
}


/* Helpers related to network byte order */

/*
    We are on x86_64 Intel (little endian) and the network byte order is ALWAYS big endian,
    so we are essentially switching bytes inside a short
*/
short htons(short input) {
    short output = 0;

    *((char*)&output) = *((char*)&input + 1);
    *((char*)&output + 1) = *((char*)&input);

    return output;
}

/*
    Here we also basically switch two bytes
*/
short ntohs(short value) {
    return htons(value);
}


/* Wrapper functions for various syscalls */
int my_fork() {
    return (int)(ssize_t) syscall5(
        (void*)57,
        (void*)NULL,
        (void*)NULL,
        (void*)NULL,
        (void*)NULL,
        (void*)NULL
    );
}


int my_socket(int family, int type, int protocol) {
    return (int)(ssize_t)syscall5(
        (void*)41,
        (void*)(ssize_t)family,
        (void*)(ssize_t)type,
        (void*)(ssize_t)protocol,
        (void*)NULL,
        (void*)NULL
    );
}


int my_bind(int fd, struct sockaddr* umyaddr, int addrlen) {
    return (int)(ssize_t)syscall5(
        (void*)49,
        (void*)(ssize_t)fd,
        (void*)umyaddr,
        (void*)(ssize_t)addrlen,
        (void*)NULL,
        (void*)NULL
    );
}


int my_listen(int fd, int backlog) {
    return (int)(ssize_t)syscall5(
        (void*)50,
        (void*)(ssize_t)fd,
        (void*)(ssize_t)backlog,
        (void*)NULL,
        (void*)NULL,
        (void*)NULL
    );
}


int my_accept(int fd, struct sockaddr* upeer_sockaddr, int* upeer_addrlen) {
    return (int)(ssize_t)syscall5(
        (void*)43,
        (void*)(ssize_t)fd,
        (void*)upeer_sockaddr,
        (void*)upeer_addrlen,
        (void*)NULL,
        (void*)NULL
    );
}


int my_setsockopt(int fd, int level, int optname, void* optval, int optlen) {
    return (int)(ssize_t)syscall5(
        (void*)54,
        (void*)(ssize_t)fd,
        (void*)(ssize_t)level,
        (void*)(ssize_t)optname,
        (void*)optval,
        (void*)(ssize_t)optlen
    );
}


int my_write(int fd, const char* buf, size_t count) {
    return (int)(ssize_t)syscall5(
        (void*)1,
        (void*)(ssize_t)fd,
        (void*)buf,
        (void*)count,
        (void*)NULL,
        (void*)NULL
    );
}


ssize_t my_recvfrom(int fd, void* ubuf, size_t size, unsigned int flags, struct sockaddr* addr, int* addr_len) {
    return (ssize_t)sys_recvfrom(
        (void*)(ssize_t)fd,
        (void*)ubuf,
        (void*)size,
        (void*)(ssize_t)flags,
        (void*)addr,
        (void*)addr_len
    );
}


/* Some even higher-level wrapper functions for syscalls */
void my_puts(const char* msg) {
    my_write(stdout, msg, strlen(msg));
    my_write(stdout, "\n", 1);
}


ssize_t my_recv(int fd, void* ubuf, size_t size, unsigned int flags) {
    return my_recvfrom(fd, ubuf, size, flags, NULL, NULL);
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
    server.sin_port = htons(5555);

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

    while((new_socket = my_accept(socket_desc, (struct sockaddr*)&client, (socklen_t*)&c))) {
        my_puts("client connected");

        const char* msg = "connected, starting separate thread handler...\n";

        my_write(new_socket, msg, strlen(msg));

        int pid = my_fork();

        if (pid < 0) {
            my_puts("fork failed");
            return 1;
        } else if (pid == 0) {
            connection_handler(new_socket);
            return 0;
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

    while((read_size = my_recv(sock, client_message, 2000, 0)) > 0) {
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
