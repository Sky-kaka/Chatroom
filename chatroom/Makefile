CC = cc
OBJS := server.o client.o
CFLAGS := -g -Wall -O

all: server client
server: server.o
	$(CC)  $<  -o $@  $(CFLAGS)

client: client.o
	$(CC)  $<  -o $@  $(CFLAGS)
$(OBJS):%.o:%.c

.PHONY:clean
clean:
	@rm -rf *.o server client








