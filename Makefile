CC= mpicxx
RM= rm

OBJS=   img.o png.o server.o client.o main.o
TARG=   vortex
LIBS=   $(shell sdl-config --libs)
CFLAGS= $(shell sdl-config --cflags) -g

#------------------------------------------------------------------------------

.cpp.o :
	$(CC) $(CFLAGS) -c $<

$(TARG) : $(OBJS)
	$(CC) $(CFLAGS) -o $(TARG) $(OBJS) $(LIBS)

clean :
	$(RM) $(TARG) $(OBJS)

#------------------------------------------------------------------------------

