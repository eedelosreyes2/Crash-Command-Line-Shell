bin=crash

# Set the following to '0' to disable log messages:
debug=0

CFLAGS += -Wall -g -DDEBUG=$(debug)
LDFLAGS +=

src=history.c shell.c tokenizer.c queue.c
obj=$(src:.c=.o)

$(bin): $(obj) 
	$(CC) $(CFLAGS) $(LDFLAGS) $(obj) -o $@

shell.o: shell.c shell.h history.h debug.h tokenizer.h 
history.o: history.c history.h shell.h queue.h
tokenizer.o: tokenizer.c tokenizer.h
queue.o: queue.c queue.h history.h

clean: 
	rm -f $(bin) $(obj)


# Tests --

test: $(bin) ./tests/run_tests 
	./tests/run_tests $(run)

testupdate: testclean test

./tests/run_tests:
	git submodule update --init --remote

testclean:
	rm -rf tests
