CC=gcc
CFlags=-I.
DEPS = linkedlist.h config.h workspaces.h

%.o: %c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

tilewm: mytilewm.c
	gcc -g -Wall -o tilewm mytilewm.c linkedlist.c workspaces.c -lX11
