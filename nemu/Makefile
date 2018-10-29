NAME = nemu
INC_DIR += ./include
BUILD_DIR ?= ./build

ifeq ($(SHARE), 1)
SO = -so
SO_CFLAGS = -fPIC -D_SHARE=1
SO_LDLAGS = -shared -fPIC
endif

PRE_DIR ?= $(BUILD_DIR)/pre$(SO)
OBJ_DIR ?= $(BUILD_DIR)/obj$(SO)
BINARY ?= $(BUILD_DIR)/$(NAME)$(SO)

include Makefile.git

.DEFAULT_GOAL = app

# Compilation flags
CC = gcc
LD = gcc
PR = gcc
INCLUDES  = $(addprefix -I, $(INC_DIR))
CFLAGS   += -O2 -MMD -Wall -Werror -ggdb3 $(INCLUDES) -fomit-frame-pointer
CFLAGS   += -DDIFF_TEST_QEMU

# Files to be compiled
SRCS = $(shell find src/ -name "*.c")
PRES = $(SRCS:src/%.c=$(PRE_DIR)/%.c)
OBJS = $(SRCS:src/%.c=$(OBJ_DIR)/%.o)

# Compilation patterns
$(PRE_DIR)/%.c: src/%.c
	@echo + PR $<
	@mkdir -p $(dir $@)
	@$(PR) $(CFLAGS) $(SO_CFLAGS) -E -c -o $@ $<

$(OBJ_DIR)/%.o: src/%.c
	@echo + CC $<
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $(SO_CFLAGS) -c -o $@ $<


# Depencies
-include $(OBJS:.o=.d)

# Some convinient rules

.PHONY: app run clean
app: $(BINARY)

override ARGS ?= -l $(BUILD_DIR)/nemu-log.txt
override ARGS += -d $(NEMU_HOME)/tools/qemu-diff/build/qemu-so

# Command to execute NEMU
NEMU_EXEC := $(BINARY) $(ARGS)

pre: $(PRES)
	@echo "Done pre-compilation."

$(BINARY): $(OBJS)
	$(call git_commit, "compile")
	@echo + LD $@
	@$(LD) -O2 -rdynamic $(SO_LDLAGS) -o $@ $^ -lSDL2 -lreadline -ldl

run: $(BINARY)
	$(call git_commit, "run")
	clear
	$(NEMU_EXEC)

gdb: $(BINARY)
	$(call git_commit, "gdb")
	clear
	gdb -s $(BINARY) --args $(NEMU_EXEC)

clean: 
	-rm -rf $(BUILD_DIR)
	$(MAKE) -C tools/gen-expr clean
	$(MAKE) -C tools/qemu-diff clean
