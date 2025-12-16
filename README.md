## Multithreaded TCP/IP echo server without libc (x86_64 Linux)

#### Running
- Simply run 'make && ./server' and application should start listening on port 5555


#### Known issues
- Changing optimization flag to -O2 or -O3 breaks the application (bind failed)
