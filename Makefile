
TARG=	vortex
OBJS=	opengl.o \
	image.o  \
	star.o   \
	galaxy.o \
	status.o \
	shared.o \
	client.o \
	server.o \
	script.o \
	main.o

#------------------------------------------------------------------------------

CC= mpicc
RM= rm

CFLAGS= $(shell sdl-config --cflags) -g
INCDIR= -I$(HOME)/include
LIBDIR= -L$(HOME)/lib

SDL_LIBS= $(shell sdl-config --libs)
LUA_LIBS= -llua -llualib
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

client.o: opengl.h glext.h shared.h status.h client.h
galaxy.o: opengl.h glext.h galaxy.h star.h
image.o: opengl.h glext.h image.h
main.o: server.h client.h
node.o: opengl.h glext.h
opengl.o: opengl.h glext.h
script.o: server.h status.h script.h
server.o: opengl.h glext.h shared.h status.h server.h star.h
shared.o: shared.h
star.o: opengl.h glext.h image.h star.h
status.o: opengl.h glext.h
viewer.o: opengl.h glext.h
