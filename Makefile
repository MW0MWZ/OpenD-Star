#
# OpenD-Star - Open Source D-Star (AMBE) Vocoder Library
#
# Makefile for building the library and test tools
#

# Compiler settings
CXX = g++
CC = gcc
CXXFLAGS = -O3 -std=c++11 -Wall -fPIC
CFLAGS = -O3 -Wall -fPIC

# Include paths
INCLUDES = -I. -Idecoder

# Library paths
LDFLAGS = -lm

# Platform detection
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    # macOS
    SHARED_EXT = dylib
    SHARED_FLAGS = -dynamiclib -install_name @rpath/libopendstar.$(SHARED_EXT)
else
    # Linux
    SHARED_EXT = so
    SHARED_FLAGS = -shared -Wl,-soname,libopendstar.$(SHARED_EXT).1
endif

# Output files
STATIC_LIB = libopendstar.a
SHARED_LIB = libopendstar.$(SHARED_EXT)
TEST_TOOL = dstar_codec

# Source files
OPENDSTAR_SRCS = opendstar.cpp

# Decoder sources (from mbelib-neo)
# D-Star AMBE (3600x2400) only
DECODER_SRCS = decoder/mbelib.c \
               decoder/mbe_adaptive.c \
               decoder/mbe_unvoiced_fft.c \
               decoder/ambe3600x2400.c \
               decoder/ambe_common.c \
               decoder/ecc.c \
               decoder/ecc_const.c \
               decoder/pffft.c \
               decoder/fftpack.c

# Object files
OPENDSTAR_OBJS = $(OPENDSTAR_SRCS:.cpp=.o)
DECODER_OBJS = $(DECODER_SRCS:.c=.o)

ALL_OBJS = $(OPENDSTAR_OBJS) $(DECODER_OBJS)

# Default target
all: $(STATIC_LIB) $(SHARED_LIB) $(TEST_TOOL)

# Static library
$(STATIC_LIB): $(ALL_OBJS)
	ar rcs $@ $^

# Shared library
$(SHARED_LIB): $(ALL_OBJS)
	$(CXX) $(SHARED_FLAGS) -o $@ $^ $(LDFLAGS)

# Test tool (statically linked)
$(TEST_TOOL): dstar_codec.cpp $(STATIC_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $< $(STATIC_LIB) $(LDFLAGS)

# Compile rules
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -Wno-unused-but-set-variable -c $< -o $@

# Clean
clean:
	rm -f $(ALL_OBJS) $(STATIC_LIB) $(SHARED_LIB) $(TEST_TOOL)
	rm -f opendstar.o dstar_codec.o
	rm -f decoder/*.o

# Install (to /usr/local by default)
PREFIX ?= /usr/local
install: $(STATIC_LIB) $(SHARED_LIB)
	install -d $(PREFIX)/lib
	install -d $(PREFIX)/include
	install -m 644 $(STATIC_LIB) $(PREFIX)/lib/
	install -m 755 $(SHARED_LIB) $(PREFIX)/lib/
	install -m 644 opendstar.h $(PREFIX)/include/

# Uninstall
uninstall:
	rm -f $(PREFIX)/lib/$(STATIC_LIB)
	rm -f $(PREFIX)/lib/$(SHARED_LIB)
	rm -f $(PREFIX)/include/opendstar.h

.PHONY: all clean install uninstall
