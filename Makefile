PREFIX=/usr/local/Electro

#------------------------------------------------------------------------------

#TARG=	electro
TARG=	electro-mpi
OBJS=	src/opengl.o   \
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
	src/star.o     \
	src/main.o

DEPS= $(OBJS:.o=.d)

#------------------------------------------------------------------------------

CC= mpicc
RM= rm -f

SDL_LIBS= $(shell sdl-config --libs)
LUA_LIBS= -llua -llualib
PNG_LIBS= -lpng -lz -lm

#CFLAGS= -g -Wall $(shell sdl-config --cflags) -DNDEBUG
CFLAGS= -g -Wall $(shell sdl-config --cflags) -DMPI -DNDEBUG
INCDIR= -I$(HOME)/include -I/usr/include/lua
LIBDIR= -L$(HOME)/lib

ifeq ($(shell uname), Darwin)
	LIBS= $(SDL_LIBS) $(LUA_LIBS) $(PNG_LIBS) -lvorbisfile
else
	LIBS= $(SDL_LIBS) $(LUA_LIBS) $(PNG_LIBS) -lvorbisfile -lGL -lGLU
endif

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

#------------------------------------------------------------------------------

include $(DEPS)
