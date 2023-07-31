CFLAG = -Wall -g -I include -MD

SRCS = $(wildcard src/*.c)
OBJS = $(patsubst src/%.c, obj/%.o, ${SRCS})		       
DEPS = $(OBJS:.o=.d)

all:flow

-include $(DEPS) #這行如果放在 all 之前，修改 .h 檔後，不會偵測到改變

flow: $(OBJS)
	gcc $(CFLAG) -o $@ $^ -lcrypto lib/libhiredis.a

obj/%.o: src/%.c
	gcc $(CFLAG) -c $< -o $@


.PHONY:	clean run

clean:
	rm -f flow obj/*.o obj/*.d

run:
	./flow 0.0.0.0 1223
