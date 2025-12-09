CC 		= gcc
CFLAGS 	= -Wall -Wextra -Werror -std=c11 -O2 -MMD -MP $(SDL2_CFLAGS)

SDL2_CFLAGS := $(shell pkg-config --cflags sdl2 2>/dev/null || sdl2-config --cflags 2>/dev/null)
SDL2_LIBS   := $(shell pkg-config --libs   sdl2 2>/dev/null || sdl2-config --libs   2>/dev/null)
GFX_LIB     := $(shell pkg-config --libs SDL2_gfx 2>/dev/null || echo -lSDL2_gfx)

# Ajout des libs manquantes
SDL2_TTF    := $(shell pkg-config --libs SDL2_ttf   2>/dev/null || echo -lSDL2_ttf)
SDL2_IMG    := $(shell pkg-config --libs SDL2_image 2>/dev/null || echo -lSDL2_image)
SDL2_MIX    := $(shell pkg-config --libs SDL2_mixer 2>/dev/null || echo -lSDL2_mixer)

LDFLAGS		=
LDLIBS 		= $(SDL2_LIBS) $(GFX_LIB) $(SDL2_TTF) $(SDL2_IMG) $(SDL2_MIX) -lm

APP = picross
SRC = main.c fonctions.c
OBJS = $(SRC:.c=.o)

$(APP): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

-include $(OBJS:.o=.d)

run: $(APP)
	./$(APP)

clean:
	rm -f $(OBJS) $(OBJS:.o=.d) $(APP)
