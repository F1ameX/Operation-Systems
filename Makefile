TARGET = proxy.out

SRCS = main.c \
       proxy/proxy.c \
       cache/cache.c \
       threadpool/threadpool.c \
       logger/logger.c


CC = gcc
CFLAGS = -g -Wall
LIBS = -lpthread
INCLUDE_DIR = "."

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) $(SRCS) $(LIBS) -o $(TARGET)

clean:
	rm -f $(TARGET) *.o