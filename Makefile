CC = gcc
CFLAGS = -I.
LDFLAGS = -pthread
OBJS_P = parent.o ProcMonit.o
OBJS_C = child.o
DEPS = ProcMonit.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: parent child

parent: $(OBJS_P)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

child: $(OBJS_C)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

# clean objects - $(RM) is rm -f by default
clean:
	$(RM) $(OBJS_P) $(OBJS_C) parent child
