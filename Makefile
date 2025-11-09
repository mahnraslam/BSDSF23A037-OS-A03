  # --- Variables for Directories ---
SRCDIR = src
INCDIR = include
OBJDIR = obj
BINDIR = bin

# --- Compiler and Flags ---
CC = gcc
CFLAGS = -g -Wall -I$(INCDIR)

# --- File Lists (Explicit) ---
# We list our .c files manually to avoid errors
SOURCES = $(SRCDIR)/main.c \
          $(SRCDIR)/execute.c \
          $(SRCDIR)/shell.c

# We list our .h file manually
DEPS = $(INCDIR)/shell.h

# We list our .o files manually
OBJS = $(OBJDIR)/main.o \
       $(OBJDIR)/execute.o \
       $(OBJDIR)/shell.o

# Name of the final program, inside the bin/ folder
# This is the line that was missing before!
TARGET = $(BINDIR)/shell


# --- Rules (The "Recipes") ---

# The first rule is the default goal. "make" will run this.
.PHONY: all
all: $(TARGET)

# Rule to build the final target (the executable)
# This rule "depends" on all the object files (.o files)
# The target is "$(TARGET)", which expands to "bin/shell"
$(TARGET): $(OBJS)
	@mkdir -p $(BINDIR)
	# This is the linking command
	# $@ means "the target" (bin/shell)
	# $^ means "all the dependencies" (all the .o files)
	$(CC) $(CFLAGS) -o $@ $^

# This is the "Pattern Rule" for building .o files from .c files
# It says: "To make any file like 'obj/%.o', you need its 'src/%.c' file"
$(OBJDIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	@mkdir -p $(OBJDIR)
	# This is the compilation command
	# -c = compile only, don't link
	# $@ means "the target" (e.g., obj/main.o)
	# $< means "the first dependency" (e.g., src/main.c)
	$(CC) $(CFLAGS) -c -o $@ $<

# Rule to clean up the project
.PHONY: clean
clean:
	# -rf = recursively, force. Deletes folders and files.
	rm -rf $(OBJDIR) $(BINDIR)