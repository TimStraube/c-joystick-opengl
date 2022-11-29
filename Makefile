CC = g++
NAME = main
SOURCE = src
TARGET = target
LDLIBS = -lglfw -lGLU -lGL -lXrandr -lXxf86vm -lXi -lXinerama -lX11 -lrt -ldl -lassimp

prog:
	${CC} -pthread -o ${TARGET}/${NAME} src/${NAME}.cpp src/glad.c ${LDLIBS}
run:
	./target/${NAME}
clean:
	rm -f target/*

