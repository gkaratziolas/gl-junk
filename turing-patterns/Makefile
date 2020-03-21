CC=g++
LIBS = -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl
INCLUDES = -I../glad/include -I../glm -I.
CFLAGS = $(LIBS) $(INCLUDES) -std=c++11
OBJ=tp

make: tp.cpp
	$(CC) -o $(OBJ) tp.cpp ../glad/src/glad.c $(CFLAGS) 
clean:
	rm $(OBJ)
