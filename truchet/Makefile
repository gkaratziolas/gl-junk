CC=g++
LIBS = -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl
INCLUDES = -I../glad/include -I../glm
CFLAGS = $(LIBS) $(INCLUDES)
OBJ=truchet

make: truchet.cpp
	$(CC) -o $(OBJ) truchet.cpp ../glad/src/glad.c $(CFLAGS) 
clean:
	rm $(OBJ)
