0;10;1c0;10;1c0;10;1c0;10;1call : player player.o ringmaster ringmaster.o helper.o potato.o

player : player.o potato.h helper.o helper.h potato.o
	gcc -o player player.o potato.h helper.o helper.h potato.o

player.o : player.c
	gcc -c player.c

ringmaster : ringmaster.o potato.h helper.o helper.h potato.o
	gcc -o ringmaster ringmaster.o potato.h helper.o helper.h potato.o

ringmaster.o : ringmaster.c
	gcc -c ringmaster.c

helper.o : helper.c helper.h
	gcc -c helper.c helper.h

potato.o : potato.c potato.h
	gcc -c potato.c potato.h

clean:
	rm -f *~ *.o *.h.gch

clobber:
	rm -f *~ *.o
