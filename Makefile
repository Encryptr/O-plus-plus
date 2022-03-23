## ----------------------------------------------------------------------
## Opp Unix Makefile
## ----------------------------------------------------------------------

CXX = gcc
EXE = o++
BUILD_DIR = build
TYPE = DEBUG
SRC_DIR = ./opp

SOURCES  =	$(SRC_DIR)/util/util.c \
			$(SRC_DIR)/memory/gc.c \
			$(SRC_DIR)/memory/memory.c \
			$(SRC_DIR)/lexer/lexer.c \
			$(SRC_DIR)/parser/parser.c \
			$(SRC_DIR)/code/code.c \
			$(SRC_DIR)/vm/vm.c \
			$(SRC_DIR)/opp.c

SRC_OBJS = $(addprefix $(BUILD_DIR)/, $(addsuffix .o, $(basename $(notdir $(SOURCES)))))

DEPS = $(SRC_OBJS:.o=.d)
CXXFLAGS += -O2 -Wall -std=c11 -pedantic-errors -Wno-newline-eof

ifeq ($(TYPE), RELEASE)
	LINKER += -static -s -static-libgcc -static-libstdc++ -mwindows
endif

.PHONY: all clean directory

all: directory compile
	@echo Build Complete!

compile: $(EXE)

directory:
	@mkdir -p $(BUILD_DIR)

clean:
	@rm -r $(BUILD_DIR)

# Build Opp Source
$(BUILD_DIR)/util.o:$(SRC_DIR)/util/util.c
	$(CXX) $(CXXFLAGS) -MMD -MP -c -o $@ $<

$(BUILD_DIR)/memory.o:$(SRC_DIR)/memory/memory.c
	$(CXX) $(CXXFLAGS) -MMD -MP -c -o $@ $<

$(BUILD_DIR)/lexer.o:$(SRC_DIR)/lexer/lexer.c
	$(CXX) $(CXXFLAGS) -MMD -MP -c -o $@ $<

$(BUILD_DIR)/parser.o:$(SRC_DIR)/parser/parser.c
	$(CXX) $(CXXFLAGS) -MMD -MP -c -o $@ $<

$(BUILD_DIR)/vm.o:$(SRC_DIR)/vm/vm.c
	$(CXX) $(CXXFLAGS) -MMD -MP -c -o $@ $<

$(BUILD_DIR)/code.o:$(SRC_DIR)/vm/code.c
	$(CXX) $(CXXFLAGS) -MMD -MP -c -o $@ $<

$(BUILD_DIR)/gc.o:$(SRC_DIR)/memory/gc.c
	$(CXX) $(CXXFLAGS) -MMD -MP -c -o $@ $<

$(BUILD_DIR)/opp.o:$(SRC_DIR)/opp.c
	$(CXX) $(CXXFLAGS) -MMD -MP -c -o $@ $<

# Link Together
$(EXE): $(SRC_OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LINKER) 

-include $(DEPS)