CXX = g++
OBJS = apriori.o
TARGET = apriori
CXXFLAGS = -O2 -m64 --std=c++11
 
.SUFFIXES : .cpp .o
 
all : $(TARGET)
 
$(TARGET): $(OBJS)
	$(CXX) -o $@ $(OBJS) $(CXXFLAGS)
 
clean :
	rm -f $(OBJS) $(TARGET)
