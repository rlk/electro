CC= cc
RM= rm

OBJS=   opengl.o image.o star.o galaxy.o viewer.o main.o
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

galaxy.o: opengl.h glext.h galaxy.h viewer.h star.h
image.o: opengl.h glext.h image.h
main.o: opengl.h glext.h viewer.h galaxy.h
node.o: opengl.h glext.h
star.o: opengl.h glext.h star.h
viewer.o: opengl.h glext.h
