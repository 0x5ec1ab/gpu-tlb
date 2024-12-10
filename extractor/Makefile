CXX = g++ 
CXXFLAGS = -g -std=c++11
SRCS = main.cpp mem-dump.cpp page-tab.cpp page-dir.cpp page-map.cpp page.cpp
OBJS = $(SRCS:.cpp=.o)
BIN = extractor

all: $(BIN)

$(BIN): $(OBJS)
	$(CXX) -o $@ $^
	rm -f $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
clean:
	rm -f $(OBJS) $(BIN)

.PHONY: all clean

