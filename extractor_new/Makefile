# 编译器
CXX = g++

# 编译选项
CXXFLAGS = -Wall -g -std=c++11

# 目标文件
OBJECTS = entry.o pde.o extractor.o

# 可执行文件
EXEC = extractor

# 默认目标
all: $(EXEC)

# 链接目标文件生成可执行文件
$(EXEC): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# 编译各个源文件生成目标文件
entry.o: entry.cpp entry.h
	$(CXX) $(CXXFLAGS) -c entry.cpp

pde.o: pde.cpp pde.h entry.h
	$(CXX) $(CXXFLAGS) -c pde.cpp

extractor.o: extractor.cpp pde3.h pde2.h pde1.h pde0.h pde.h pte.h entry.h vm_area_struct.h
	$(CXX) $(CXXFLAGS) -c extractor.cpp

# 清理生成的文件
clean:
	rm -f $(OBJECTS) $(EXEC)

# 伪目标
.PHONY: all clean
