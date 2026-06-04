CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -g
TARGET  = dispatcher
SRCS    = dispatcher.c queue.c scheduler.c
OBJS    = $(SRCS:.c=.o)

.PHONY: all run1 run2 run3 run4 runall clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run1: $(TARGET)
	@echo "\n>>> Running FCFS <<<\n"
	./$(TARGET) 1

run2: $(TARGET)
	@echo "\n>>> Running Round Robin <<<\n"
	./$(TARGET) 2

run3: $(TARGET)
	@echo "\n>>> Running Priority Scheduling <<<\n"
	./$(TARGET) 3

run4: $(TARGET)
	@echo "\n>>> Running SJF <<<\n"
	./$(TARGET) 4

runall: $(TARGET)
	./$(TARGET) 1
	./$(TARGET) 2
	./$(TARGET) 3
	./$(TARGET) 4

clean:
	rm -f $(TARGET) $(OBJS)
