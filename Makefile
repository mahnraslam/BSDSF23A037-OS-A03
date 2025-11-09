 # --- Variables for Directories ---
SRCDIR = src
INCDIR = include
OBJDIR = obj
BINDIR = bin

# --- Compiler and Flags ---
CC = gcc
CFLAGS = -g -Wall -I$(INCDIR)
# *** ADD THIS LINE for the readline library ***
LDFLAGS = -lreadline

# --- File Lists (Explicit) ---
# We list our .c files manually
SOURCES = $(SRCDIR)/main.c \
          $(SRCDIR)/execute.c \
          $(SRCDIR)/shell.c

# We list our .h file manually
DEPS = $(INCDIR)/shell.h

# --- File Lists (Generated) ---
OBJS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# Name of the final program, inside the bin/ folder
TARGET = $(BINDIR)/shell


# --- Rules (The "Recipes") ---

.PHONY: all
all: $(TARGET)

# Rule to build the final target (the executable)
$(TARGET): $(OBJS)
	@mkdir -p $(BINDIR)
	# *** MODIFY THIS LINE to include $(LDFLAGS) ***
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# This is the "Pattern Rule" for building .o files from .c files
$(OBJDIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Rule to clean up the project
.PHONY: clean
clean:
	rm -rf $(OBJDIR) $(BINDIR)