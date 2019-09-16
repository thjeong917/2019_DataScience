CXX = g++
OBJS = decision_tree.o
TARGET = dt
CXXFLAGS = -O2 --std=c++11
 
.SUFFIXES : .cpp .o
 
all : $(TARGET)
 
$(TARGET): $(OBJS)
	$(CXX) -o $@ $(OBJS) $(CXXFLAGS)
 
clean :
	rm -f $(OBJS) $(TARGET)
