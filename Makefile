CC = gcc

BLOCKS_DIR_NAME = "./Assets/Blocks/"
BUILDINGS_DIR_NAME = "./Assets/Buildings/"
BLOCKS_DIR_PATH = $(shell realpath -- $(BLOCKS_DIR_NAME))
BUILDINGS_DIR_PATH = $(shell realpath -- $(BUILDINGS_DIR_NAME))

FILES = $(wildcard ./src/*.c)
OBJECTS = $(patsubst ./src/%.c, ./build/%.o, $(FILES))
LDFLAGS = -lc -lm $(shell pkg-config --libs cglm) \
		  	$(shell pkg-config --static --libs glew) \
			$(shell pkg-config --libs glfw3) \
			$(shell pkg-config --libs assimp) \

CPPFLAGS =
CFLAGS = -Wall -Wextra -Wshadow -D'BLOCKS_DIR_PATH="$(BLOCKS_DIR_PATH)"' -D'BUILDINGS_DIR_PATH="$(BUILDINGS_DIR_PATH)"'

.PHONY: update urun

info:
	@echo "files: "
	@echo $(FILES)
	@echo $(OBJECTS)
	@echo $(LDFLAGS)
	@echo "blocks: $(BLOCKS_DIR_PATH)"
	@echo "buildings: $(BUILDINGS_DIR_PATH)"

# build/main.o: src/shaders.h
# 	@echo "shader was updated"

build/%.o: src/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

main: $(OBJECTS)
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o ./build/$@

flags = 

run: main
	./build/main $(flags)

# blocks: includeFile.c
# 	gcc includeFile.c -o ./build/includeFile && ./build/includeFile | tee src/blocks.h

shaders: makeShaders.c
	gcc makeShaders.c -o ./build/makeShaders && ./build/makeShaders | tee src/shaders.h

debug: CFLAGS += -O0 -ggdb
debug: main
	@echo ${DARGS}
	gdb -ex "set debuginfod enabled off" -ex "set args $(DARGS)" -ex run ./build/main
