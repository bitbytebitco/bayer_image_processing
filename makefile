CC = gcc
CFLAGS = -Wall -Wextra -O2
OBJ = bayer_filter.o demosaic.o utils.o
DEPS = utils.h

all: bayer_filter demosaic

bayer_filter: bayer_filter.o utils.o
	$(CC) $(CFLAGS) -o bayer_filter bayer_filter.o utils.o

demosaic: demosaic.o utils.o
	$(CC) $(CFLAGS) -o demosaic demosaic.o utils.o

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

clean: 
	rm -f *.o *.bmp bayer_filter demosaic

