CFLAGS=-D__LINUX__ -Wall -O2 
LIBS = 

all: jpegenc_test

jpegenc_test: main.o
	$(CC) main.o $(LIBS) -o jpegenc_test 

main.o: main.cpp
	$(CXX) $(CFLAGS) -c main.cpp

clean:
	rm -rf *.o jpegenc_test
