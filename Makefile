CC= cc
RM= rm

OBJS=   galaxy.o viewer.o main.o
TARG=   vortex
LIBS=   $(shell sdl-config --libs)
CFLAGS= $(shell sdl-config --cflags) -g

#------------------------------------------------------------------------------

.c.o :
	$(CC) $(CFLAGS) -c $<

$(TARG) : $(OBJS)
	$(CC) $(CFLAGS) -o $(TARG) $(OBJS) $(LIBS)

clean :
	$(RM) $(TARG) $(OBJS)

#------------------------------------------------------------------------------

