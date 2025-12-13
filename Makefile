
build: main


asm: asm.s
	gcc -c asm.s -o asm

main: main.c asm
	gcc \
    -O2 \
    -s \
    -fno-unwind-tables \
    -fno-asynchronous-unwind-tables \
    -nostdlib \
    asm \
    main.c \
    -o server
