CXX = g++
OBJS = recommender.o
TARGET = recommender
CXXFLAGS = -O2 --std=c++11
 
.SUFFIXES : .cpp .o
 
all : $(TARGET)
 
$(TARGET): $(OBJS)
	$(CXX) -o $@ $(OBJS) $(CXXFLAGS)
 
clean :
	rm -f $(OBJS) $(TARGET)
