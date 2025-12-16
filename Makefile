
build: main

main: main.c asm.s Makefile
	gcc \
    -O0 \
    -nostdlib \
    main.c \
    asm.s \
    -o server
