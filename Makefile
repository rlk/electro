
TARG=	vortex
OBJS=	opengl.o \
	image.o  \
	star.o   \
	node.o   \
	galaxy.o \
	status.o \
	shared.o \
	client.o \
	server.o \
	script.o \
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

client.o: opengl.h glext.h star.h galaxy.h shared.h status.h client.h
galaxy.o: opengl.h glext.h galaxy.h star.h
image.o: opengl.h glext.h image.h
main.o: server.h client.h
node.o: opengl.h glext.h galaxy.h node.h
opengl.o: opengl.h glext.h
script.o: server.h status.h shared.h script.h
server.o: opengl.h glext.h shared.h status.h galaxy.h script.h server.h
server.o: star.h
shared.o: status.h shared.h
star.o: opengl.h glext.h image.h star.h
status.o: opengl.h glext.h
viewer.o: opengl.h glext.h
