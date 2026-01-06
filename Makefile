all: xxcc

CFLAGS ?= -O0 -Wall -g -std=c++11 -fshort-wchar -I ./src/include -Wno-writable-strings

CPP := clang++

XXCC_SOURCES := $(shell find ./src -name "*.cpp")

XXCC_OBJECTS := $(patsubst ./%.cpp, ./out/%.o, $(XXCC_SOURCES))

xxcc: $(XXCC_OBJECTS)
	@$(CPP) $(XXCC_OBJECTS) -o ./out/$@ -static
	@echo "LINK ./out/$@"

./out/%.o: %.cpp
	@mkdir -p $(@D)
	@$(CPP) $(CFLAGS) -c $< -o $@
	@echo "CPP $< -> $@"

.PHONY: clean
clean:
	rm -rf ./out