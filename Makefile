#Makefile for arith (Comp 40 asignment 4)
#
# Includes build rules for 40image, test_compress and test_bitpack
#
# New syntax rules added:  wildcards (%.o, %.c) and special variables:
# $@ - the target of the rule
# $< - the first dependency
# $^ - all dependencies

############## Variables ###############

CC = gcc # The compiler being used

# Updating include path to use Comp 40 .h files and CII interfaces
IFLAGS = -I. -I/comp/40/include -I/usr/sup/cii40/include/cii

# Compile flags
# Set debugging information, allow the c99 standard,
# max out warnings, and use the updated include path
CFLAGS = -g -std=c99 -Wall -Wextra -Werror -pedantic $(IFLAGS)
# -Wfatal-errors flag removed -- this flag makes the compiler stop after only
# one error is revealed. This behavior is annoying. Norman doesn't like it.

# Linking flags
# Set debugging information and update linking path
# to include course binaries and CII implementations
LDFLAGS = -g -L/comp/40/lib64 -L/usr/sup/cii40/lib64

# Libraries needed for linking
# Both programs need cii40 (Hanson binaries) and *may* need -lm (math)
# Only brightness requires the binary for pnmrdr.
LDLIBS = -l40locality -lcii40 -lnetpbm -lcii40 -lm -lrt -larith40

# Collect all .h files in your directory.
# This way, you can never forget to add
# a local .h file in your dependencies.
#
# This bugs Mark, who dislikes false dependencies, but
# he agrees with Noah that you'll probably spend hours 
# debugging if you forget to put .h files in your 
# dependency list.
INCLUDES = $(shell echo *.h)

############### Rules ###############

all: 40image


## Compile step (.c files -> .o files)

# To get *any* .o file, compile its .c file with the following rule.
%.o: %.c $(INCLUDES)
	$(CC) $(CFLAGS) -c $< -o $@


## Linking step (.o -> executable program)

40image: compress40.o 40image.o compress_helper.o a2blocked.o uarray2.o uarray2b.o bitpack.o
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

40image-6: compress40.o 40image.o compress_helper.o a2blocked.o uarray2.o uarray2b.o bitpack.o
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)


clean:
	rm -f   test_compress test_bitpack 40image *.o     

