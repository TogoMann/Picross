CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -std=c11 -O2
LDLIBS = -lm

all = picross
SRC = main.c fonctions.c
OBJ = $(SRC:.c=.o)

$(all): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(all)