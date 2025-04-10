CXX = g++
CXXFLAGS = -std=c++11 -O2 -Wall -MMD -MP

ROOTCFLAGS  = $(shell root-config --cflags)
ROOTLIBS    = $(shell root-config --libs)

OBJDIR = obj
BINDIR = bin

TARGETS = $(BINDIR)/UserAna $(BINDIR)/makeroot $(BINDIR)/OnlineHist

# Source files
SRCS1  = makeroot.cc
SRCS2  = UserAna.cc Tree.cc Hist.cc
SRCS3  = OnlineHist.cc Tree.cc Hist.cc

OBJS1  = $(addprefix $(OBJDIR)/, $(SRCS1:.cc=.o))
OBJS2  = $(addprefix $(OBJDIR)/, $(SRCS2:.cc=.o))
OBJS3  = $(addprefix $(OBJDIR)/, $(SRCS3:.cc=.o))

LIBS1  = -larchive $(ROOTLIBS)
LIBS2  = $(ROOTLIBS)
LIBS3  = $(ROOTLIBS)

TARGET1 = $(BINDIR)/makeroot
TARGET2 = $(BINDIR)/UserAna
TARGET3 = $(BINDIR)/OnlineHist

all: $(TARGET1) $(TARGET2) $(TARGET3)

$(TARGET1): $(OBJS1) | $(BINDIR)
	$(CXX) $(CXXFLAGS) $(ROOTCFLAGS) -o $@ $^ $(LIBS1)

$(TARGET2): $(OBJS2) | $(BINDIR)
	$(CXX) $(CXXFLAGS) $(ROOTCFLAGS) -o $@ $^ $(LIBS2)

$(TARGET3): $(OBJS3) | $(BINDIR)
	$(CXX) $(CXXFLAGS) $(ROOTCFLAGS) -o $@ $^ $(LIBS3)

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

-include $(OBJS1:.o=.d) $(OBJS2:.o=.d) $(OBJS3:.o=.d)
