
TARG=	vortex
OBJS=	opengl.o \
	image.o  \
	star.o   \
	galaxy.o \
	viewer.o \
	shared.o \
	client.o \
	server.o \
	script.o \
	main.o

#------------------------------------------------------------------------------

CC= /usr/local/mpich-1.2.6/bin/mpicc
RM= rm

CFLAGS= $(shell sdl-config --cflags) -g
INCDIR=
LIBDIR=

SDL_LIBS= $(shell sdl-config --libs)
LUA_LIBS= -llua -lualib
PNG_LIBS= -lpng -lz -lm

ifeq ($(shell uname), Darwin)
	LIBS= $(SDL_LIBS) $(LUA_LIBS) $(PNG_LIBS)
else
	LIBS= $(SDL_LIBS) $(LUA_LIBS) $(PNG_LIBS) -lGL -lGLU
endif

#------------------------------------------------------------------------------

.c.o :
	$(CC) $(CFLAGS) -c $< $(INCDIR)

$(TARG) : $(OBJS)
	$(CC) $(CFLAGS) -o $(TARG) $(OBJS) $(LIBDIR) $(LIBS)

clean :
	$(RM) $(TARG) $(OBJS)

depend :
	makedepend -Y *.c 2> /dev/null


#------------------------------------------------------------------------------
# DO NOT DELETE

client.o: opengl.h glext.h shared.h client.h
galaxy.o: opengl.h glext.h galaxy.h viewer.h star.h
image.o: opengl.h glext.h image.h
main.o: server.h client.h
node.o: opengl.h glext.h
opengl.o: opengl.h glext.h
script.o: server.h script.h
server.o: opengl.h glext.h shared.h server.h star.h
shared.o: shared.h
star.o: opengl.h glext.h image.h star.h
viewer.o: opengl.h glext.h
