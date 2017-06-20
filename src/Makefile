CC=gcc
CFlags=-I.
DEPS = linkedlist.h config.h workspaces.h

%.o: %c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

tilewm: carlywm.c
	gcc -g -Wall -o carlywm carlywm.c linkedlist.c workspaces.c -lX11
