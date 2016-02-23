# use "make -f Makefile" to build, ignore warnings
# the compiler: gcc for C program, define as g++ for C++
CC = gcc -std=c99

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
#CFLAGS  = -g -Wall

# the build target executable:
TARGET = shell

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

clean:
	$(RM) $(TARGET)