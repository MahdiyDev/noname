CC = gcc
CFLAGS=-O0 -g
LIBS=-lm

vm.out: main.o chunk.o debug.o value.o vm.o
	$(CC) $^ -o $@ $(LIBS)

main.o: main.c chunk.h common.h value.h debug.h vm.h
	$(CC) $(CFLAGS) -c $< -o $@

chunk.o: chunk.c ../libs/dynamic_array.h value.h chunk.h common.h
	$(CC) $(CFLAGS) -c $< -o $@

debug.o: debug.c debug.h chunk.h common.h value.h
	$(CC) $(CFLAGS) -c $< -o $@

value.o: value.c ../libs/dynamic_array.h value.h
	$(CC) $(CFLAGS) -c $< -o $@

vm.o: vm.c vm.h chunk.h common.h value.h debug.h
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f *.o