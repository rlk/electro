PREFIX=/usr/local/Electro
SDL_CONFIG=sdl-config

#------------------------------------------------------------------------------

# To build in cluster mode: "make MPI=1".

ifdef MPI
	CC=     mpicc
	TARG=   electro-mpi
	CFLAGS= $(shell $(SDL_CONFIG) --cflags) -g -Wall -DMPI -DNDEBUG
else
	CC=     cc
	TARG=   electro
	CFLAGS= $(shell $(SDL_CONFIG) --cflags) -g -Wall -ansi -pedantic
endif

#------------------------------------------------------------------------------

SDLLIB= $(shell $(SDL_CONFIG) --libs) -lSDLmain
LUALIB= -llua -llualib
IMGLIB= -ljpeg -lpng -lz -lm
OGGLIB= -lvorbisfile

# Assume the Fink tree is available under OSX and GL is in a framework.

ifeq ($(shell uname), Darwin)
	INCDIR= -I/sw/include
	LIBDIR= -L/sw/lib
	OGLLIB=
else
	OGLLIB= -lGL -lGLU
endif

# Include Lua, if it exists.

ifneq ($(wildcard /usr/include/lua),)
	INCDIR += -I/usr/include/lua
endif

# If user include or lib directories exist, assume dependancies are there.

ifneq ($(wildcard $(HOME)/include),)
	INCDIR += -I$(HOME)/include
endif
ifneq ($(wildcard $(HOME)/lib),)
	LIBDIR += -L$(HOME)/lib
endif

#------------------------------------------------------------------------------

LIBS= $(SDLLIB) $(LUALIB) $(IMGLIB) $(OGGLIB) $(OGLLIB) -lm

OBJS=	src/opengl.o   \
	src/glyph.o    \
	src/matrix.o   \
	src/utility.o  \
	src/joystick.o \
	src/viewport.o \
	src/console.o  \
	src/buffer.o   \
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

#------------------------------------------------------------------------------

.c.o :
	$(CC) $(CFLAGS) $(INCDIR) -c -o $@ $<

$(TARG) : $(OBJS)
	$(CC) $(CFLAGS) -o $(TARG) $(OBJS) $(LIBDIR) $(LIBS)

clean :
	rm -f $(TARG) $(OBJS)

install : $(TARG)
	cp $(TARG) $(PREFIX)/bin
	cp config/* $(PREFIX)/config

#------------------------------------------------------------------------------
