CC = gcc

FILES = $(wildcard ./src/*.c)
OBJECTS = $(patsubst ./src/%.c, ./build/%.o, $(FILES))
LDFLAGS = -lc -lm $(shell pkg-config --libs cglm) \
		  	$(shell pkg-config --static --libs glew) \
			$(shell pkg-config --libs glfw3) \
			$(shell pkg-config --libs assimp) \

CPPFLAGS =
CFLAGS = -Wall -Wextra -Wshadow

.PHONY: update urun

db: 
	@echo $(FILES)
	@echo $(OBJECTS)
	@echo $(LDFLAGS)

# build/main.o: src/shaders.h
# 	@echo "shader was updated"

build/%.o: src/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<;

main: $(OBJECTS)
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o ./build/$@

flags = 

run: main
	./build/main $(flags)

blocks: includeFile.c
	gcc includeFile.c -o ./build/includeFile && ./build/includeFile | tee src/blocks.h

shaders: makeShaders.c
	gcc makeShaders.c -o ./build/makeShaders && ./build/makeShaders | tee src/shadersDef.h

debug: CFLAGS += -O0 -ggdb
debug: main
	@echo ${DARGS}
	gdb -ex "set debuginfod enabled off" -ex "set args $(DARGS)" -ex run ./build/main
