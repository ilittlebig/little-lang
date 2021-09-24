TARGET_EXEC ?= parser

BIN_DIR ?= ./bin
SRC_DIRS ?= ./src

SRCS := $(shell find $(SRC_DIRS) -name *.c)
OBJS := $(SRCS:%=$(BIN_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP

$(BIN_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BIN_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	$(RM) -r $(BIN_DIR)

run:
	$(BIN_DIR)/parser examples/main.lil
	as --32 $(BIN_DIR)/assembly.asm -o $(BIN_DIR)/a.o
	ld -m elf_i386 $(BIN_DIR)/a.o -o $(BIN_DIR)/a

-include $(DEPS)

MKDIR_P ?= mkdir -p
