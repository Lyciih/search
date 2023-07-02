all:
	gcc main.c -o flow

.PHONY:	clean

clean:
	rm -f flow

run:
	./flow
