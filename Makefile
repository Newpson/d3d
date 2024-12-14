CFLAGS += -std=c23 -fopenmp
LDFLAGS += -lSDL2 -lm -fopenmp

all: d3d

d3d: main.o algebra.o memory.o scene.o d3d.o

.PHONY: all clean check
clean:
	rm -f *.o d3d
