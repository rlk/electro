PREFIX = /usr/local/Electro

#------------------------------------------------------------------------------

# To build in cluster mode: "make MPI=1".

ifdef MPI
	CC     = mpicc
	TARG   = electro-mpi
	CFLAGS =  -g -Wall -DMPI -DNDEBUG
else
	CC     = cc
	TARG   = electro
	CFLAGS = -O2 -Wall -ansi -pedantic
endif

# To build with trackdAPI: "make TRACKD=1"

ifdef TRACKD
	LIBS   += -Wl,-rpath $(HOME)/lib -ltrackdAPI_c
	CFLAGS += -DTRACKD
endif

#------------------------------------------------------------------------------

# Assume the Fink tree is available under OSX and GL is in a framework.

ifeq ($(shell uname), Darwin)
	INCDIR = -I/sw/include
	LIBDIR = -L/sw/lib
	OGLLIB =
	SDL_CONFIG = /sw/bin/sdl-config
else
	OGLLIB = -lGL -lGLU
	SDL_CONFIG = /usr/bin/sdl-config
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

CFLAGS += $(shell $(SDL_CONFIG) --cflags)
SDLLIB  = $(shell $(SDL_CONFIG) --libs) -lSDLmain
LUALIB  = -llua -llualib
IMGLIB  = -ljpeg -lpng -lz -lm
OGGLIB  = -lvorbisfile

LIBS += $(SDLLIB) $(LUALIB) $(IMGLIB) $(OGGLIB) $(OGLLIB)

OBJS =	src/version.o  \
	src/opengl.o   \
	src/glyph.o    \
	src/matrix.o   \
	src/utility.o  \
	src/tracker.o  \
	src/joystick.o \
	src/frustum.o  \
	src/display.o  \
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
# If Subversion is installed then report the revision number in the source.

ifneq ($(shell which svnversion),)

src/version.c : FORCE
	echo -n 'const char *version(void) { const char *str = "' \
	                         > src/version.c
	svnversion -n .         >> src/version.c
	echo '"; return str; }' >> src/version.c

else

src/version.c : FORCE
	echo 'const char *version(void) { return ""; }' > src/version.c

endif

#------------------------------------------------------------------------------

FORCE :
