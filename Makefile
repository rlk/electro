
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

CC= cc
RM= rm

MPI_CFLAGS= -Wno-long-long -DUSE_STDARG -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 \
	-DHAVE_UNISTD_H=1 -DHAVE_STDARG_H=1 -DUSE_STDARG=1 -DMALLOC_RET_VOID=1

SDL_LIBS= $(shell sdl-config --libs)
LUA_LIBS= -llua -llualib
PNG_LIBS= -lpng -lz -lm
MPI_LIBS= -lmpich

MPI_LIBDIR= -L/usr/local/mpich-1.2.6/lib
MPI_INCDIR= -I/usr/local/mpich-1.2.6/include

CFLAGS= -g -ansi -pedantic -Wall $(MPI_CFLAGS) $(shell sdl-config --cflags)
INCDIR= $(MPI_INCDIR) -I$(HOME)/include
LIBDIR= $(MPI_LIBDIR) -L$(HOME)/lib

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
client.o: opengl.h glext.h shared.h client.h camera.h galaxy.h star.h
galaxy.o: opengl.h glext.h galaxy.h shared.h star.h
image.o: opengl.h glext.h image.h
main.o: server.h client.h
node.o: opengl.h glext.h galaxy.h node.h
opengl.o: opengl.h glext.h
script.o: shared.h opengl.h glext.h server.h camera.h script.h
server.o: opengl.h glext.h shared.h server.h script.h camera.h galaxy.h
server.o: star.h
shared.o: image.h opengl.h glext.h shared.h camera.h
star.o: opengl.h glext.h shared.h image.h star.h
