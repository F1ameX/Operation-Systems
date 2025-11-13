TARGET = proxy.out

SRCS = main.c \
       threadpool/threadpool.c

CC = gcc
CFLAGS = -g -Wall
LIBS = -lpthread
INCLUDE_DIR = "."

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) $(SRCS) $(LIBS) -o $(TARGET)

clean:
	rm -f $(TARGET) *.o