#define stdout 1
#define NULL 0


void connection_handler(int sock);


// assembly-defined syscall's

void* sys_recvfrom(
        void* arg1,
        void* arg2,
        void* arg3,
        void* arg4,
        void* arg5,
        void* arg6
);

void* syscall5(
        void* number,
        void* arg1,
        void* arg2,
        void* arg3,
        void* arg4,
        void* arg5
);



// wrappers

void* syscall4(
        void* number,
        void* arg1,
        void* arg2,
        void* arg3,
        void* arg4
) { syscall5(number, arg1, arg2, arg3, arg4, NULL); };

void* syscall3(
        void* number,
        void* arg1,
        void* arg2,
        void* arg3
) { syscall5(number, arg1, arg2, arg3, NULL, NULL); };

void* syscall2(
        void* number,
        void* arg1,
        void* arg2
) { syscall5(number, arg1, arg2, NULL, NULL, NULL); };

void* syscall1(
        void* number,
        void* arg1
) { syscall5(number, arg1, NULL, NULL, NULL, NULL); };

void* syscall0(
        void* number
) { syscall5(number, NULL, NULL, NULL, NULL, NULL); };

// typedefs

typedef long unsigned int size_t;
typedef long ssize_t;

typedef unsigned long int in_addr_t;
typedef unsigned short in_port_t;

/* Size of these typedefs were got from call to sizeof() on my machine, this may vary */
/* socklen_t: 4, sa_family_t: 2 */
typedef unsigned int socklen_t;
typedef unsigned short sa_family_t;

/**/


// defines

#define AF_INET 2
#define SOCK_STREAM 1
#define SOL_SOCKET 1
#define SO_REUSEADDR 2
#define INADDR_ANY ((in_addr_t) 0x00000000)


//structs

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


// libc stuff

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


// network functions

/*
    We are on Intel (little endian) and the network byte order is big endian,
    so we are essentially switching bytes inside a short
*/
short htons(short input) {
    short output = 0;

    *((char*)&output) = *((char*)&input + 1);
    *((char*)&output + 1) = *((char*)&input);

    return output;
}

/*
    Same thing as htons
*/
short ntohs(short value) {
    return htons(value);
}


// wrappers for various syscalls

int my_fork() {
    return syscall0(57);
}


int my_socket(int family, int type, int protocol) {
    return syscall3(41, family, type, protocol);
}


int my_bind(int fd, struct sockaddr* umyaddr, int addrlen) {
    return syscall3(49, fd, umyaddr, addrlen);
}


int my_listen(int fd, int backlog) {
    return syscall2(50, fd, backlog);
}


int my_accept(int fd, struct sockaddr* upeer_sockaddr, int* upeer_addrlen) {
    return syscall3(43, fd, upeer_sockaddr, upeer_addrlen);
}


int my_setsockopt(int fd, int level, int optname, void* optval, int optlen) {
    return syscall5(54, fd, level, optname, optval, optlen);
}


int my_write(unsigned int fd, const char* buf, size_t count) {
    return syscall3(1, fd, buf, count);
}


ssize_t my_recvfrom(int fd, void* ubuf, size_t size, unsigned int flags, struct sockaddr* addr, int* addr_len) {
    //return syscall6(45, fd, ubuf, size, flags, addr, addr_len);
    //return syscall5(45, fd, ubuf, size, flags, addr);
    return sys_recvfrom(fd, ubuf, size, flags, addr, addr_len);
}


// some even higher-level wrappers for syscalls
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
