PREFIX=/usr/local/Electro

#------------------------------------------------------------------------------

ifdef MPI
	CC= mpicc
	CFLAGS= -g -Wall $(shell sdl-config --cflags) -DMPI -DNDEBUG
	TARG= electro-mpi
else
	CC= cc
	CFLAGS= -g -Wall $(shell sdl-config --cflags)
	TARG= electro
endif

#------------------------------------------------------------------------------

SDLLIB= $(shell sdl-config --libs) -lSDLmain
LUALIB= -llua -llualib
IMGLIB= -ljpeg -lpng -lz -lm
OGGLIB= -lvorbisfile

# Assume the Fink tree is available under OSX.

ifeq ($(shell uname), Darwin)
	INCDIR= -I/sw/include
	LIBDIR= -L/sw/lib
	OGLLIB=
else
	OGLLIB= -lGL -lGLU
endif

# If Lua is not found globally, assume it is in the user's home directory.

ifneq ($(shell ls /usr/include/lua),)
	INCDIR += -I/usr/include/lua
else
	INCDIR += -I$(HOME)/include
	LIBDIR += -L$(HOME)/lib
endif

LIBS= $(SDLLIB) $(LUALIB) $(IMGLIB) $(OGGLIB) $(OGLLIB) -lm

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
