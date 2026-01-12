CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

SRC = src/main.cpp \
      src/QRCode.cpp \
      src/DataEncoder.cpp \
      src/ErrorCorrection.cpp

TARGET = qrcode

all: $(TARGET)

$(TARGET):
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET) *.o qrcode_output.txt
