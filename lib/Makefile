CC = gcc  # C compiler
CFLAGS = -fPIC -Wno-switch 
LDFLAGS = -shared -Wall # linking flags
RM = rm -f  # rm command
TARGET_LIB = libexample.so  # target lib

SRCS =  opp_lib_example.c ../Opp/src/util/error.c ../Opp/src/env/enviroment.c ../Opp/src/lexer/lexer.c ../Opp/src/parser/parser.c ../Opp/src/interpreter/interpreter.c ../Opp/src/parser/ostdlib.c ../Opp/src/util/dl_init.c
OBJS = $(SRCS:.c=.o)

.PHONY: all
all: ${TARGET_LIB}

$(TARGET_LIB): $(OBJS)
	$(CC) ${LDFLAGS} -o $@ $^

$(SRCS:.c=.d):%.d:%.c
	$(CC) $(CFLAGS) -MM $< >$@

include $(SRCS:.c=.d)

.PHONY: clean
clean:
	-${RM} ${TARGET_LIB} ${OBJS} $(SRCS:.c=.d)