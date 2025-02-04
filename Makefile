CC = gcc
CFLAGS=-O0 -g
# CFLAGS=-O2
LIBS=-lm

noname.out: parser.o lexer.o expression.o \
			statement.o interpreter.o environment.o \
			libs/temp_alloc.o libs/string.o libs/hash_table.o
	$(CC) $^ -o $@ $(LIBS)

parser.o: parser.c libs/error.h
	$(CC) $(CFLAGS) -c $< -o $@

lexer.o: lexer.c lexer.h
	$(CC) $(CFLAGS) -c $< -o $@

expression.o: expression.c expression.h
	$(CC) $(CFLAGS) -c $< -o $@

statement.o: statement.c statement.h
	$(CC) $(CFLAGS) -c $< -o $@

interpreter.o: interpreter.c interpreter.h
	$(CC) $(CFLAGS) -c $< -o $@

environment.o: environment.c environment.h
	$(CC) $(CFLAGS) -c $< -o $@

temp_alloc.o: temp_alloc.c temp_alloc.h
	$(CC) $(CFLAGS) -c $< -o $@

##### BUILDING LIBS #####
libs/string.o: libs/string.c libs/string.h
	$(CC) $(CFLAGS) -c $< -o $@

libs/hash_table.o: libs/hash_table.c libs/hash_table.h
	$(CC) $(CFLAGS) -c $< -o $@

libs/temp_alloc.o: libs/temp_alloc.c libs/temp_alloc.h
	$(CC) $(CFLAGS) -c $< -o $@
### BUILDING LIBS END ###

build_dir:
	mkdir -p $(BUILD_OBJ_DIR)

.PHONY: clean
clean:
	rm -f *.o
	rm -f libs/*.o
