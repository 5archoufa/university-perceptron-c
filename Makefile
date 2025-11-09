# === Compiler & Directories ===
CC = gcc
SRC_DIR = src
BUILD_DIR = build
TARGET = $(BUILD_DIR)/perceptron

# === Common Flags ===
CFLAGS = -Wall -O2 -Iinclude -g -DDEBUG_COLLIDERS
LDFLAGS = -lcglm -lm

# === Platform-specific ===
ifeq ($(OS),Windows_NT)
    TARGET := $(TARGET).exe
    LDFLAGS += -lwinmm -lgdi32 -lopengl32 -luser32 -Wl,-subsystem,console -lglfw3
    MKDIR = mkdir
    RM = del /Q
else
    LDFLAGS += -ldl -lpthread -lglfw
    MKDIR = mkdir -p
    RM = rm -f
endif

# === Helper function: recursive wildcard ===
rwildcard = $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

# === Files ===
SRCS := $(call rwildcard,$(SRC_DIR)/,*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

# === Build rules ===
$(TARGET): $(OBJS)
	@if not exist "$(BUILD_DIR)" $(MKDIR) "$(BUILD_DIR)"
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@if not exist "$(dir $@)" $(MKDIR) "$(dir $@)"
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

.PHONY: clean run
clean:
	-$(RM) $(TARGET) $(OBJS) $(DEPS) 2>nul || true

run: $(TARGET)
	./$(TARGET)
