CC = gcc
FLAG = -std=c11 -Wno-switch 

# Uncomment if want to init port
RAYLIB = ./tools/raylib_port.c 
FLAG_RAY = -lraylib

FILES = ./util/error.c ./env/enviroment.c ./lexer/lexer.c ./parser/parser.c ./interpreter/interpreter.c ./parser/ostdlib.c
MAIN = opp.c

EXE = o++

all:
	$(CC) $(FLAG) $(FILES) $(MAIN) -o o++

raylib:
	$(CC) $(FLAG) $(FLAG_RAY) $(RAYLIB) $(FILES) $(MAIN) -o o++

clean:
	rm o++
