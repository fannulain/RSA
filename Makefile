CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -Isrc
LDFLAGS = -lbcrypt

SRCS = src/BigNum.cpp src/RSA.cpp src/main.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = rsa_app.exe

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET)
