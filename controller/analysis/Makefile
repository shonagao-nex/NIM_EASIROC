CXX = g++
CXXFLAGS = -std=c++11 -O2 -Wall

# Get ROOT flags via root-config
ROOTCFLAGS  = $(shell root-config --cflags)
ROOTLIBS    = $(shell root-config --libs)

# Output binary name
TARGET = offline

# Source files
SRCS = offline.cc

# Libraries
LIBS = -larchive $(ROOTLIBS)

# Default target
all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(ROOTCFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f $(TARGET)

.PHONY: all clean

