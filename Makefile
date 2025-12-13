
build: main

main: main.c asm.s
	gcc \
    -O0 \
    -s \
    -fno-unwind-tables \
    -fno-asynchronous-unwind-tables \
    -nostdlib \
    main.c \
    asm.s \
    -o server
