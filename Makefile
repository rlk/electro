
TARG=	electro
OBJS=	src/opengl.o   \
	src/joystick.o \
	src/viewport.o \
	src/buffer.o   \
	src/shared.o   \
	src/server.o   \
	src/client.o   \
	src/entity.o   \
	src/camera.o   \
	src/sprite.o   \
	src/object.o   \
	src/light.o    \
	src/pivot.o    \
	src/script.o   \
	src/image.o    \
	src/sound.o    \
	src/main.o

DEPS= $(OBJS:.o=.d)

#------------------------------------------------------------------------------

CC= mpicc
RM= rm -f

SDL_LIBS= $(shell sdl-config --libs)
LUA_LIBS= -llua50 -llualib50
PNG_LIBS= -lpng -lz -lm

CFLAGS= -g -Wall $(shell sdl-config --cflags)
#CFLAGS= -g -Wall $(shell sdl-config --cflags) -DMPI -DNDEBUG
INCDIR= -I$(HOME)/include -I/usr/include/lua50
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

#------------------------------------------------------------------------------

include $(DEPS)
