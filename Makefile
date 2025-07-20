CXX      := g++
CXXFLAGS := -std=c++14 -O2 -Wall -Iinclude `pkg-config --cflags sqlite3 jsoncpp`
LDFLAGS  := `pkg-config --libs sqlite3 jsoncpp` -lcpp-httplib -pthread
SRCS     := src/main.cpp src/db.cpp src/server.cpp src/create.cpp
OBJS     := $(SRCS:.cpp=.o)
TARGET   := vokabeltrainer

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)