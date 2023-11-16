CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

cmm: $(OBJS)
	$(CC) -o cmm $(OBJS) $(LDFLAGS)

$(OBJS): cmm.h

test: cmm 
	./test.sh

clean:
	rm -f cmm *.o *~ tmp*

.PHONY: test clean
