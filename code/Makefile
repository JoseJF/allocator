#######################################
#  Project main makefile
#######################################
# TARGET: name of the output file
TARGET = test

# Provide compiler
COMPILER=g++

# SOURCES: list of input source sources
SOURCES = main.cpp \
	  allocator.cpp \
	  mgmt.cpp \
	  vector.cpp

# INCLUDES: list of includes, by default, use Includes directory
INCLUDES = -I./

# LIBS
#LIBPATH=-L
#LIBS=-l

# OUTDIR: directory to use for output
OUTDIR = build

# LD_SCRIPT: linker script
LD_SCRIPT = $(TARGET).ld

# define flags
CFLAGS = -std=c++17 
CFLAGS += -O0 
CFLAGS += -Wall -pedantic 
DBGFLAGS = -g -ggdb

# tools
CC = g++
LD = g++
RM      = rm -f
MKDIR   = mkdir -p


# list of object files, placed in the build directory regardless of source path
OBJS = $(patsubst %.c, %.o, $(SOURCES))

$(OUTDIR)/%.o: $(SOURCES)
	$(CC) $(CFLAGS) $< -o $(OUTDIR) $(LIBPATH) $(LIBS)

$(OUTDIR)/$(TARGET).out: $(OBJS)
	${MKDIR} ${OUTDIR}
	$(LD) $(INCLUDES) $(DBGFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	-$(RM) $(OUTDIR)/*
	-$(RM) *.o *.out *.d

exe:
	./$(OUTDIR)/$(TARGET).out

gdb:
	gdb ./$(OUTDIR)/$(TARGET).out

hardclean:
	-$(RM) $(OUTDIR)/*
	-$(RM) *.o *.out *.d *.*~ *~

.PHONY: all clean
