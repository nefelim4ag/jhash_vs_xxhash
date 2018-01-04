CFLAGS=-O2 -Wall -Werror

default all: hash_test

cityhash.o: cityhash.c
	$(CC) $(CFLAGS) -c $? -o $@

xxhash.o: xxhash.c
	$(CC) $(CFLAGS) -c $? -o $@

hash_test.o: hash_test.c
	$(CC) $(CFLAGS) -c $? -o $@

hash_test: xxhash.o hash_test.o cityhash.o
	$(CC) $(CFLAGS) -o $@ $?


clean: ## Cleanup
	rm -fv *.o

help: ## Show help
	@fgrep -h "##" $(MAKEFILE_LIST) | fgrep -v fgrep | sed -e 's/\\$$//' | sed -e 's/##/\t/'
