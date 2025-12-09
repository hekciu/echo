
typedef long int my_ssize_t;
typedef long unsigned int my_size_t;


void* syscall5(
        void* number,
        void* arg1,
        void* arg2,
        void* arg3,
        void* arg4,
        void* arg5
);


static my_ssize_t write(int fd, const char* data, my_size_t nbytes) {
    return (my_ssize_t)
    syscall5(
        (void*) 1,            /* SYS_write, call number 1 */
        (void*) (my_ssize_t) fd,
        (void*) data,
        (void*) nbytes,
        0,                    /* Ignored */
        0                     /* Ignored */
    );
}


int main(int argc, char* argv[]) {
    write(1, "hello, world\n", 13);

    return 0;
}
