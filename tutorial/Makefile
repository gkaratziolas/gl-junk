CC=g++
LIBS = -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl
INCLUDES = -I../glad/include
CFLAGS = $(LIBS) $(INCLUDES)
OBJ=a

make: a.cpp
	$(CC) -o $(OBJ) a.cpp ../glad/src/glad.c $(CFLAGS) 
clean:
	rm $(OBJ)
