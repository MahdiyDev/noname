CC = gcc
CFLAGS=-O0 -g
LIBS=-lm

vm.out: main.o chunk.o debug.o value.o vm.o compiler.o scanner.o string.o
	$(CC) $^ -o $@ $(LIBS)

main.o: main.c vm.h chunk.h common.h value.h ../libs/string.h ../libs/dynamic_array.h
	$(CC) $(CFLAGS) -c $< -o $@

chunk.o: chunk.c ../libs/dynamic_array.h value.h chunk.h common.h
	$(CC) $(CFLAGS) -c $< -o $@

debug.o: debug.c debug.h chunk.h common.h value.h
	$(CC) $(CFLAGS) -c $< -o $@

value.o: value.c ../libs/dynamic_array.h value.h
	$(CC) $(CFLAGS) -c $< -o $@

vm.o: vm.c vm.h chunk.h common.h value.h compiler.h debug.h
	$(CC) $(CFLAGS) -c $< -o $@

compiler.o: compiler.c compiler.h scanner.h common.h
	$(CC) $(CFLAGS) -c $< -o $@

scanner.o: scanner.c scanner.h
	$(CC) $(CFLAGS) -c $< -o $@

##### BUILDING LIBS #####
string.o: ../libs/string.c ../libs/string.h ../libs/dynamic_array.h
	$(CC) $(CFLAGS) -c $< -o $@
### BUILDING LIBS END ###

.PHONY: clean
clean:
	rm -f *.o
