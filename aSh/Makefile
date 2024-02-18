# Default optimization level
O ?= 2

all: aSh

%.o: %.cc aSh.hh
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -O$(O) -o $@ -c $<

aSh: aSh.o aShhelpers.o
	$(CXX) $(CXXFLAGS) -O$(O) -o $@ $^ $(LDFLAGS) $(LIBS)

clean:
	rm -f aSh *.o

.PHONY: all clean
