CXX = g++
CXXFLAGS = -std=c++11 -O2 -Wall

ROOTCFLAGS  = $(shell root-config --cflags)
ROOTLIBS    = $(shell root-config --libs)

OBJDIR = obj
BINDIR = bin

TARGETS = $(BINDIR)/draw $(BINDIR)/makeroot

# Source files
SRCS1  = makeroot.cc
SRCS2  = draw.cc Tree.cc Hist.cc

OBJS1  = $(addprefix $(OBJDIR)/, $(SRCS1:.cc=.o))
OBJS2  = $(addprefix $(OBJDIR)/, $(SRCS2:.cc=.o))

LIBS1  = -larchive $(ROOTLIBS)
LIBS2  = $(ROOTLIBS)

TARGET1 = $(BINDIR)/makeroot
TARGET2 = $(BINDIR)/draw

all: $(TARGET1) $(TARGET2)

$(TARGET1): $(OBJS1) | $(BINDIR)
	$(CXX) $(CXXFLAGS) $(ROOTCFLAGS) -o $@ $^ $(LIBS1)

$(TARGET2): $(OBJS2) | $(BINDIR)
	$(CXX) $(CXXFLAGS) $(ROOTCFLAGS) -o $@ $^ $(LIBS2)

$(OBJDIR)/%.o: %.cc | $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(ROOTCFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

# Clean
clean:
	rm -rf $(OBJDIR)/* $(BINDIR)/*

.PHONY: all clean
