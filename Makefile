 #-----------------------------------------------------------------------
# A Self-Explanatory Makefile for the C Shell Project
#
# How to use:
#   - Type `make` (or `make all`) to build the program.
#   - Type `make clean` to remove all built files.
#   - Type `make run` to build (if needed) and run the shell.
#-----------------------------------------------------------------------

# --- 1. Settings & Configuration (The "Variables") ---
# We define variables here so we can easily change them later.

# The C compiler we will use.
CC = gcc

# Compiler Flags:
#   -g      = Add debugging info (for GDB)
#   -Wall   = Warn about all common problems (helps find bugs!)
#   -I      = "Include". Tells GCC where to find our header files.
CFLAGS = -g -Wall -I$(INCDIR)

# Linker Flags:
#   -lreadline = Link the final program with the 'readline' library.
LDFLAGS = -lreadline

# --- 2. Directory Variables ---
# Defining our folder structure.

INCDIR = include
SRCDIR = src
OBJDIR = obj
BINDIR = bin

# --- 3. File & Target Variables ---

# The final, single, executable program we want to build.
# $(BINDIR) is replaced with 'bin', so this becomes 'bin/shell'.
TARGET = $(BINDIR)/shell

# A list of all our .c source files.
# 'make' will look in the 'src' folder for these.
SOURCES = \
    $(SRCDIR)/main.c \
    $(SRCDIR)/shell.c \
    $(SRCDIR)/execute.c

# A list of all our .h header files.
# We use this to make sure .o files are rebuilt if a header changes.
DEPS = \
    $(INCDIR)/shell.h

# This is a "magic" substitution rule.
# It automatically converts our list of .c files (in SOURCES)
# into a list of .o (object) files.
# Example: "src/main.c" becomes "obj/main.o"
OBJS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)


#-----------------------------------------------------------------------
# --- 4. The "Recipes" (The "Targets") ---
# These are the instructions for 'make' to build things.
#
# Format:
# target: dependencies
#	(TAB) command
#
# 'target'   = The file you want to build (e.g., 'bin/shell').
# 'dependencies' = The files that must exist *before* building the target.
# 'command'  = The shell command to run (MUST start with a TAB).
#-----------------------------------------------------------------------

# The '.PHONY' target tells 'make' that these are "virtual" targets,
# not actual files on disk. This prevents confusion.
.PHONY: all clean run

# "all" is the default target. It's listed first.
# Running `make` will try to build the 'all' target.
# This target *depends* on our final $(TARGET) executable.
all: $(TARGET)
	@echo "Build complete! Run with '$(BINDIR)/shell' or 'make run'"

# --- Recipe to build the final executable ---
# It depends on all the .o files (in the $(OBJS) variable).
$(TARGET): $(OBJS)
	# First, create the 'bin' directory if it doesn't exist.
	# The '@' symbol at the start tells 'make' to not print this command.
	@mkdir -p $(BINDIR)
	
	# Now, link all the .o files together into the final program.
	# '$^' is a "magic variable" that means "all the dependencies" (all the .o files)
	# '$@' is a "magic variable" that means "the target" (bin/shell)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# --- "Pattern Rule" to build .o files from .c files ---
# This is a generic recipe for *any* .o file.
# It says: "To make a file in 'obj/' ending in '.o'..."
# "...you need the file in 'src/' ending in '.c', AND you need all the headers in $(DEPS)."
$(OBJDIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	# First, create the 'obj' directory if it doesn't exist.
	@mkdir -p $(OBJDIR)
	
	# Compile (but don't link) the .c file into a .o file.
	# '-c' means "compile only".
	# '$<' is a "magic variable" that means "the first dependency" (the .c file).
	# '$@' is a "magic variable" that means "the target" (the .o file).
	$(CC) $(CFLAGS) -c -o $@ $<

# --- Recipe to run the program ---
# This depends on 'all', so 'make' will build the program first if needed.
run: all
	@echo "--- Starting Shell ---"
	./$(TARGET)
	@echo "\n--- Shell Exited ---"

# --- Recipe to clean up the project ---
clean:
	@echo "Cleaning up build files..."
	# 'rm -rf' forcefully removes the 'obj' and 'bin' directories.
	rm -rf $(OBJDIR) $(BINDIR)