 # --- Variables for Directories ---
SRCDIR = src
OBJDIR = obj
BINDIR = bin

# --- Variables for Compiler ---
CC = gcc
# We add -I$(SRCDIR) to tell gcc to "look in the src folder for header files"
CFLAGS = -g -Wall -I$(SRCDIR)

# --- Variables for Files ---

# Get all .c file names from the src directory
# $(wildcard) finds all files matching the pattern
SOURCES = $(wildcard $(SRCDIR)/*.c)

# Get the list of header files
DEPS = $(wildcard $(SRCDIR)/*.h)

# Create the list of .o files.
# This substitutes "src/%.c" with "obj/%.o"
# e.g., "src/main.c" becomes "obj/main.o"
OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SOURCES))

# Set the name of the final executable, inside the bin directory
TARGET = $(BINDIR)/shell


# --- Rules ---

# The first rule is the default.
.PHONY: all
all: $(TARGET)

# Rule to link the final executable
# It depends on all the .o files in the obj/ directory
$(TARGET): $(OBJS)
	# @mkdir -p creates the bin/ directory if it doesn't exist
	# The @ symbol stops "make" from printing the command
	@mkdir -p $(BINDIR)
	# This links all the .o files into the final program in bin/
	$(CC) $(CFLAGS) -o $@ $^

# This is the "pattern rule" to build .o files
# It says: "To make a file like 'obj/%.o', you need its 'src/%.c' file"
$(OBJDIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	# Create the obj/ directory if it doesn't exist
	@mkdir -p $(OBJDIR)
	# This compiles (but doesn't link) the .c file
	# $< is the .c file (the first dependency)
	# $@ is the .o file (the target)
	$(CC) $(CFLAGS) -c -o $@ $<

# Rule to clean up all compiled files
.PHONY: clean
clean:
	# -rf means "recursively, and force"
	# This completely removes the obj/ and bin/ directories
	rm -rf $(OBJDIR) $(BINDIR)