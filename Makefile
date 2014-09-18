
OPTS= -Wall

# To build in cluster mode: "make MPI=1".

ifdef MPI
	CC    = mpicc -DNDEBUG -DCONF_MPI
	TARG  = electro-mpi
else
	CC    = cc
	TARG  = electro
endif

#------------------------------------------------------------------------------
# Determine the target platform.

ifeq      ($(shell uname), Darwin)
	ISMACOS = true
else ifeq ($(shell uname), Linux)
	ISLINUX = true
else ifeq ($(shell uname), MINGW32_NT-6.1)
	ISMINGW = true
endif

#------------------------------------------------------------------------------
# Configure the system libraries.

ifdef ISMACOS
	SYSLIBS = -lstdc++ \
		  -framework OpenGL \
 		  -framework IOKit \
		  -framework Cocoa \
 		  -framework Carbon \
 		  -framework CoreAudio \
 		  -framework AudioUnit \
 		  -framework AudioToolbox \
 		  -framework ForceFeedback \
 		  -framework ApplicationServices
 	OPTS   += -Wno-deprecated-declarations
	LIBPATH = /usr/local/lib/ /opt/local/lib/ /usr/lib/
endif

ifdef ISLINUX
	SYSLIBS = -lGL -lGLU
	LIBPATH = /usr/local/lib/ /usr/lib/ /usr/lib/x86_64-linux-gnu/
endif

ifdef ISMINGW
	OPTS    += -static -DGLEW_STATIC
#	SYSLIBS += -mwindows
	SYSLIBS += -lopengl32 -lole32 -loleaut32 -lws2_32 -lversion -luuid -lwinmm -limm32 -lgdi32 -luser32 -lkernel32 -lcomctl32
	LIBS    += -lmingw32
	LIBPATH  = C:/MinGW/lib/
endif

#------------------------------------------------------------------------------
# Configure the dependencies

PKG_CONFIG = $(firstword $(wildcard /usr/local/bin/pkg-config \
			                  /usr/bin/pkg-config) \
			                           pkg-config)

PKGS = vorbisfile libpng ode freetype2 sdl zlib

OPTS += $(sort $(shell $(PKG_CONFIG) --cflags $(PKGS)))
LIBS += $(shell $(PKG_CONFIG) --libs $(PKGS)) -ljpeg -llua $(SYSLIBS)

#------------------------------------------------------------------------------

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
	src/terrain.o  \
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
	src/net.o      \
	src/vec.o      \
	src/main.o

SRCS= $(OBJS:.o=.c)
DEPS= $(OBJS:.o=.d)

#------------------------------------------------------------------------------

%.d : %.c
	$(CC) $(OPTS) -MM -MG -MF $@ -MT $*.o $<

%.o : %.c Makefile
	$(CC) $(OPTS) -c -o $@ $<

$(TARG) : $(OBJS) Makefile
	$(CC) $(OPTS) -o $(TARG) $(OBJS) $(LIBDIR) $(LIBS)

clean :
	rm -f $(TARG) $(OBJS) $(DEPS)

#------------------------------------------------------------------------------

-include $(DEPS)
