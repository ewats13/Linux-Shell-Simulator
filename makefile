CC=gcc
CFLAGS=
RM=/bin/rm -f

mini437sh: mini437sh_EW.c
	$(CC) $(CFLAGS) -o $@ mini437sh_EW.c chop_line.c list.c

clean:
	$(RM) mini437sh *~
