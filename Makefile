CXXFLAGS := -Wall -Werror -O3 -std=c++11
LDFLAGS := -fopenmp

BIN = reduce.out

all: $(BIN)

reduce.out: reduce.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< $(LDFLAGS)

clean:
	rm -f *~ *.out *.o
