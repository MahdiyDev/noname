CC = gcc
CFLAGS=-O0 -g
# CFLAGS=-O2
LIBS=-lm

noname.out: parser.o lexer.o expression.o interpreter.o   \
			environment.o function.o statement.o noname.o \
			hash_table.o temp_alloc.o string.o
	$(CC) $^ -o $@ $(LIBS)

parser.o: parser.c libs/dynamic_array.h libs/error.h libs/string.h \
 expression.h lexer.h libs/temp_alloc.h parser.h statement.h
	$(CC) $(CFLAGS) -c $< -o $@

lexer.o: lexer.c lexer.h libs/string.h libs/dynamic_array.h
	$(CC) $(CFLAGS) -c $< -o $@

expression.o: expression.c libs/string.h libs/dynamic_array.h \
 libs/temp_alloc.h lexer.h expression.h
	$(CC) $(CFLAGS) -c $< -o $@

interpreter.o: interpreter.c function.h lexer.h libs/string.h \
 libs/dynamic_array.h libs/error.h interpreter.h statement.h expression.h \
 libs/temp_alloc.h environment.h libs/hash_table.h
	$(CC) $(CFLAGS) -c $< -o $@

environment.o: environment.c environment.h lexer.h libs/string.h \
 libs/dynamic_array.h libs/hash_table.h libs/temp_alloc.h libs/error.h
	$(CC) $(CFLAGS) -c $< -o $@

function.o: function.c function.h lexer.h libs/string.h \
 libs/dynamic_array.h interpreter.h statement.h expression.h \
 libs/temp_alloc.h environment.h libs/hash_table.h libs/error.h
	$(CC) $(CFLAGS) -c $< -o $@

statement.o: statement.c libs/temp_alloc.h statement.h expression.h \
 lexer.h libs/string.h libs/dynamic_array.h
	$(CC) $(CFLAGS) -c $< -o $@

noname.o: noname.c libs/error.h interpreter.h statement.h expression.h \
 lexer.h libs/string.h libs/dynamic_array.h libs/temp_alloc.h parser.h
	$(CC) $(CFLAGS) -c $< -o $@

##### BUILDING LIBS #####

hash_table.o: libs/hash_table.c libs/prime.h libs/hash_table.h \
 libs/temp_alloc.h
	$(CC) $(CFLAGS) -c $< -o $@

temp_alloc.o: libs/temp_alloc.c libs/temp_alloc.h
	$(CC) $(CFLAGS) -c $< -o $@

string.o: libs/string.c libs/string.h libs/dynamic_array.h
	$(CC) $(CFLAGS) -c $< -o $@

### BUILDING LIBS END ###

build_dir:
	mkdir -p $(BUILD_OBJ_DIR)

.PHONY: clean
clean:
	rm -f *.o
	rm -f libs/*.o
