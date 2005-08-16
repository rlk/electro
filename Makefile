PREFIX = /DEMO/evl/Electro

#------------------------------------------------------------------------------

# To build in cluster mode: "make MPI=1".

ifdef MPI
	CC     = mpicc
	TARG   = electro-mpi
	CFLAGS = -O2 -Wall -DNDEBUG -DMPI
else
	CC     = cc
	TARG   = electro
	CFLAGS = -g -Wall
endif

# To build with trackdAPI: "make TRACKD=1"

ifdef TRACKD
	LIBS   += -Wl,-rpath $(HOME)/lib -ltrackdAPI_c
	CFLAGS += -DTRACKD
endif

# To build without audio: "make NAUDIO=1"

ifdef NAUDIO
	CFLAGS += -DNAUDIO
endif

# To build without networking: "make NSOCKET=1"

ifdef NSOCKET
	CFLAGS += -DNSOCKET
else
	LIBS += -lluasocket
endif

#------------------------------------------------------------------------------

# Assume the Fink tree is available under OSX and GL is in a framework.

ifeq ($(shell uname), Darwin)
	INCDIR = -I/sw/include
	LIBDIR = -L/sw/lib
	OGLLIB =
	SDL_CONFIG = /sw/bin/sdl-config
	FT2_CONFIG = /sw/lib/freetype2/bin/freetype-config
else
	OGLLIB = -lGL -lGLU
	SDL_CONFIG = /usr/bin/sdl-config
	FT2_CONFIG = /usr/bin/freetype-config
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

CFLAGS += $(shell $(SDL_CONFIG) --cflags) $(shell $(FT2_CONFIG) --cflags)
SDLLIB  = $(shell $(SDL_CONFIG) --libs) -lSDLmain
FT2LIB  = $(shell $(FT2_CONFIG) --libs)
LUALIB  = -llua -llualib
IMGLIB  = -ljpeg -lpng -lz -lm
OGGLIB  = -lvorbisfile
ODELIB  = -lode -lstdc++

LIBS += $(LUALIB) $(ODELIB) $(SDLLIB) $(FT2LIB) $(IMGLIB) $(OGGLIB) $(OGLLIB)

VERS =	src/version.o
OBJS =	src/opengl.o   \
	src/video.o    \
	src/glyph.o    \
	src/matrix.o   \
	src/vector.o   \
	src/utility.o  \
	src/tracker.o  \
	src/joystick.o \
	src/frustum.o  \
	src/display.o  \
	src/console.o  \
	src/physics.o  \
	src/buffer.o   \
	src/server.o   \
	src/client.o   \
	src/entity.o   \
	src/camera.o   \
	src/stereo.o   \
	src/galaxy.o   \
	src/sprite.o   \
	src/object.o   \
	src/string.o   \
	src/light.o    \
	src/pivot.o    \
	src/script.o   \
	src/image.o    \
	src/brush.o    \
	src/sound.o    \
	src/font.o     \
	src/node.o     \
	src/star.o     \
	src/main.o

SRCS= $(OBJS:.o=.c)
DEPS= $(OBJS:.o=.d)

#------------------------------------------------------------------------------

%.d : %.c
	$(CC) $(CFLAGS) $(INCDIR) -MM -MF $@ -MT $*.o $<

%.o : %.c
	$(CC) $(CFLAGS) $(INCDIR) -c -o $@ $<

$(TARG) : $(OBJS) $(VERS)
	$(CC) $(CFLAGS) -o $(TARG) $(OBJS) $(VERS) $(LIBDIR) $(LIBS)

clean :
	rm -f $(TARG) $(OBJS)

distclean:
	find . -name .svn | xargs rm -rf
	rm -f $(DEPS)

install : $(TARG)
	cp $(TARG) $(PREFIX)/bin
	cp config/* $(PREFIX)/config

#------------------------------------------------------------------------------
# If Subversion is available, report the revision number in the source.

ifneq ($(shell which svnversion),)

src/version.c : $(OBJS)
	echo -n 'const char *version(void) { const char *str = "' \
	                              > src/version.c
	svnversion -n . | tr -d '\n' >> src/version.c
	echo '"; return str; }'      >> src/version.c
else

src/version.c : $(OBJS)
	echo 'const char *version(void) { return ""; }' > src/version.c
endif

#------------------------------------------------------------------------------

-include $(DEPS)
