CC= g++
RM= rm

INCDIR= -I/sw/include
LIBDIR= -L/sw/lib

OBJS=   img.o png.o server.o client.o main.o
TARG=   vortex
LIBS=   $(LIBDIR) $(shell sdl-config --libs) -lpng -lz
CFLAGS= $(INCDIR) $(shell sdl-config --cflags) -g

#------------------------------------------------------------------------------

.cpp.o :
	$(CC) $(CFLAGS) -c $<

$(TARG) : $(OBJS)
	$(CC) $(CFLAGS) -o $(TARG) $(OBJS) $(LIBS)

clean :
	$(RM) $(TARG) $(OBJS)

#------------------------------------------------------------------------------

