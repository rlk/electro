PREFIX=/usr/local/Electro

#------------------------------------------------------------------------------

ifdef MPI
	CC= mpicc
	CFLAGS= -std=c99 -g -Wall $(shell sdl-config --cflags) -DMPI -DNDEBUG
	TARG= electro-mpi
else
	CC= cc
	CFLAGS= -std=c99 -g -Wall $(shell sdl-config --cflags) -DNDEBUG
	TARG= electro
endif

#------------------------------------------------------------------------------

OBJS=	src/opengl.o   \
	src/matrix.o   \
	src/utility.o  \
	src/joystick.o \
	src/viewport.o \
	src/buffer.o   \
	src/shared.o   \
	src/server.o   \
	src/client.o   \
	src/entity.o   \
	src/camera.o   \
	src/galaxy.o   \
	src/sprite.o   \
	src/object.o   \
	src/light.o    \
	src/pivot.o    \
	src/script.o   \
	src/image.o    \
	src/sound.o    \
	src/node.o     \
	src/star.o     \
	src/main.o

DEPS= $(OBJS:.o=.d)

#------------------------------------------------------------------------------

RM= rm -f

SDL_LIBS= $(shell sdl-config --libs) -lSDLmain
LUA_LIBS= -llua -llualib
PNG_LIBS= -lpng -lz -lm

ifeq ($(shell uname), Darwin)
    INCDIR= -I/sw/include -I/usr/include/lua
    LIBDIR= -L/sw/lib
else
    INCDIR= -I$(HOME)/include -I/usr/include/lua
    LIBDIR= -L$(HOME)/lib
endif

LIBS= $(SDL_LIBS) $(LUA_LIBS) $(PNG_LIBS) -lvorbisfile -lm

#------------------------------------------------------------------------------

%.d : %.c
	$(CC) $(CFLAGS) $(INCDIR) -MM -MF $@ -MT $*.o $<

%.o : %.c
	$(CC) $(CFLAGS) $(INCDIR) -c -o $@ $<

#------------------------------------------------------------------------------

$(TARG) : $(OBJS)
	$(CC) $(CFLAGS) -o $(TARG) $(OBJS) $(LIBDIR) $(LIBS)

clean :
	$(RM) $(TARG) $(OBJS) $(DEPS)

install : $(TARG)
	cp $(TARG) $(PREFIX)/bin
	cp config/* $(PREFIX)/config

#------------------------------------------------------------------------------

include $(DEPS)
