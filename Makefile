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

depend :
	makedepend -Y *.c 2> /dev/null


#------------------------------------------------------------------------------
# DO NOT DELETE

galaxy.o: opengl.h glext.h galaxy.h viewer.h png.h
main.o: opengl.h glext.h viewer.h galaxy.h
png.o: png.h opengl.h glext.h
viewer.o: opengl.h glext.h
