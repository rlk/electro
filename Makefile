CC= cc
RM= rm

OBJS=   png.o galaxy.o viewer.o main.o
TARG=   vortex
LIBS=   $(shell sdl-config --libs) -lpng -lz -lGL -lGLU -lm
#LIBS=   $(shell sdl-config --libs) -lpng -lz -lm
CFLAGS= $(shell sdl-config --cflags) -g

#------------------------------------------------------------------------------

.c.o :
	$(CC) $(CFLAGS) -c $<

$(TARG) : $(OBJS)
	$(CC) $(CFLAGS) -o $(TARG) $(OBJS) $(LIBS)

clean :
	$(RM) $(TARG) $(OBJS)

#------------------------------------------------------------------------------

