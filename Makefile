CFLAGS = -I/usr/lib64/graphene-1.0/include $(shell pkg-config --cflags gtk4 libadwaita-1 graphene-1.0)
LIBS = $(shell pkg-config --libs gtk4 libadwaita-1 graphene-1.0)

window2: window2.c
	gcc $(CFLAGS) window2.c $(LIBS) -o window2
window3: window3.c
	gcc $(CFLAGS) window3.c $(LIBS) -o window3

r: r.c
	gcc -Wall r.c -o r
s: s.c
	gcc -Wall s.c -o s
