TARGET ?= o++
SRC_DIRS ?= ./opp

SRCS := $(shell find $(SRC_DIRS) -name *.c)
OBJS := $(addsuffix .o,$(basename $(SRCS)))
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP -O1 -std=c99 -pedantic-errors -Wextra -Wno-unused-result

# valgrind --tool=memcheck --leak-check=yes --track-origins=yes 

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@ $(LOADLIBES) $(LDLIBS)
	$(RM) $(OBJS) $(DEPS)

.PHONY: clean
clean:
	$(RM) $(OBJS) $(DEPS)

-include $(DEPS)
