
TARG=	vortex
OBJS=	opengl.o \
	shared.o \
	server.o \
	client.o \
	entity.o \
	camera.o \
	sprite.o \
	object.o \
	light.o  \
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
LUA_LIBS= -llua50 -llualib50
PNG_LIBS= -lpng -lz -lm

CFLAGS= -g -Wall $(shell sdl-config --cflags)
INCDIR= -I$(HOME)/include -I/usr/include/lua50
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

camera.o: opengl.h glext.h shared.h server.h entity.h camera.h
client.o: opengl.h glext.h shared.h client.h camera.h sprite.h entity.h
client.o: galaxy.h star.h
entity.o: opengl.h glext.h shared.h server.h camera.h sprite.h object.h
entity.o: entity.h
galaxy.o: opengl.h glext.h galaxy.h shared.h star.h
image.o: opengl.h glext.h image.h
main.o: shared.h opengl.h glext.h server.h client.h
node.o: opengl.h glext.h galaxy.h node.h
object.o: opengl.h glext.h entity.h object.h
opengl.o: opengl.h glext.h
script.o: shared.h opengl.h glext.h server.h camera.h object.h sprite.h
script.o: entity.h script.h
server.o: opengl.h glext.h shared.h server.h script.h camera.h entity.h
server.o: galaxy.h star.h
shared.o: image.h opengl.h glext.h shared.h camera.h
sprite.o: opengl.h glext.h shared.h server.h entity.h sprite.h
star.o: opengl.h glext.h shared.h image.h star.h
