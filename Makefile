CC=gcc
CFLAGS=-std=c99 -Wall
LDFLAGS=-g
OBJS=output.o
SOURCES=$(OBJS:.o=.c)

output: $(OBJS)
	$(CC) $(LDFLAGS)  $(OBJS) -o $@

%.o: %.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f ./*.o
	rm -f output

new: clean dependiente
