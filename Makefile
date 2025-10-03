CC = g++
CFLAGS = -std=c++20 -I/opt/homebrew/Cellar/nlohmann-json/3.12.0/include
LDFLAGS = -lcurl
SRCS = ${wildcard src/*.cpp}
OBJ = ${SRCS:src/%.cpp=build/%.o}
TARGET = muffinlauncher

all: ${TARGET}
${TARGET}: ${OBJ}
	${CC} ${CFLAGS} $^ -o $@ ${LDFLAGS}

build/%.o: src/%.cpp | build
	${CC} ${CFLAGS} -c $< -o $@

build:
	mkdir -p build

clean:
	rm -rf build ${TARGET}

.PHONY: all clean build
