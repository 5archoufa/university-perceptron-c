CC = gcc
CFLAGS = -Wall -O2 -Iinclude -g -lX11 -lm
TARGET = $(BUILD_DIR)/perceptron
SRC_DIR = src
BUILD_DIR = build
SRCS = $(shell find $(SRC_DIR) -name '*.c')
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))
DS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.d, $(SRCS))

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

-include $(OBJS:.o=.d)

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJS) $(DS)