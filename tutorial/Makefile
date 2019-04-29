CC=g++
LIBS = -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl
INCLUDES = -I../glad/include
CFLAGS = $(LIBS) $(INCLUDES)
OBJ=a

make: basic-window.cpp
	$(CC) -o $(OBJ) basic-window.cpp ../glad/src/glad.c $(CFLAGS) 
clean:
	rm $(OBJ)
