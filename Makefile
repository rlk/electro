
TARG=	vortex
OBJS=	opengl.o \
	shared.o \
	server.o \
	client.o \
	camera.o \
	sprite.o \
	script.o \
	galaxy.o \
	node.o   \
	star.o   \
	image.o  \
	main.o

#------------------------------------------------------------------------------

CC= mpicc
RM= rm

SDL_LIBS= $(shell sdl-config --libs)
LUA_LIBS= -llua -llualib
PNG_LIBS= -lpng -lz -lm

CFLAGS= -O2 -Wall -Wno-long-long $(shell sdl-config --cflags)
INCDIR= -I$(HOME)/include
LIBDIR= -L$(HOME)/lib

ifeq ($(shell uname), Darwin)
	LIBS= $(SDL_LIBS) $(LUA_LIBS) $(PNG_LIBS) $(MPI_LIBS)
else
	LIBS= $(SDL_LIBS) $(LUA_LIBS) $(PNG_LIBS) $(MPI_LIBS) -lGL -lGLU
endif

#------------------------------------------------------------------------------

.c.o :
	$(CC) $(CFLAGS) $(INCDIR) -c $<

$(TARG) : $(OBJS)
	$(CC) $(CFLAGS) -o $(TARG) $(OBJS) $(LIBDIR) $(LIBS)

clean :
	$(RM) $(TARG) $(OBJS)

depend :
	makedepend -Y *.c 2> /dev/null


#------------------------------------------------------------------------------
# DO NOT DELETE

camera.o: opengl.h glext.h shared.h server.h camera.h
client.o: opengl.h glext.h shared.h client.h camera.h sprite.h galaxy.h
client.o: star.h
galaxy.o: opengl.h glext.h galaxy.h shared.h star.h
image.o: opengl.h glext.h image.h
main.o: server.h client.h
node.o: opengl.h glext.h galaxy.h node.h
opengl.o: opengl.h glext.h
script.o: shared.h opengl.h glext.h server.h camera.h sprite.h script.h
server.o: opengl.h glext.h shared.h server.h script.h camera.h sprite.h
server.o: galaxy.h star.h
shared.o: image.h opengl.h glext.h shared.h camera.h
sprite.o: opengl.h glext.h shared.h server.h camera.h sprite.h
star.o: opengl.h glext.h shared.h image.h star.h
