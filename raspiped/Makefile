CC=clang
CFLAGS=-Wall -ggdb
SRCS=raspiped.c server.c
OBJS=$(patsubst %.c, %.o, $(SRCS))
HDRS=globals.h
TARGET=raspiped.out

$(TARGET) : $(OBJS) tags
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

$(OBJS) : $(SRCS) $(HDRS)

tags: $(SRCS) $(HDRS)
	ctags -R .

clean:
	rm -f *.o *.out
