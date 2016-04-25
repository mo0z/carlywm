tilewm: mytilewm.c
	gcc -g -Wall -std=c99 -o tilewm mytilewm.c linkedlist.c workspaces.c config.h -lX11
