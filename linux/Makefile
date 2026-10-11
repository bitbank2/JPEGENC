CFLAGS=-D__LINUX__ -Wall -O2 
LIBS = 

all: jpegenc

jpegenc: main.o JPEGENC.o jpegenc_core.o
	$(CC) main.o JPEGENC.o jpegenc_core.o $(LIBS) -o jpegenc

c_api_test: c_api_test.o jpegenc_core.o
	$(CC) c_api_test.o jpegenc_core.o $(LIBS) -o c_api_test

main.o: main.cpp
	$(CXX) $(CFLAGS) -c main.cpp

c_api_test.o: c_api_test.c
	$(CC) $(CFLAGS) -c c_api_test.c

JPEGENC.o: ../src/JPEGENC.cpp ../src/JPEGENC.h
	$(CXX) $(CFLAGS) -c ../src/JPEGENC.cpp

jpegenc_core.o: ../src/jpegenc.c ../src/jpegenc.inl ../src/JPEGENC.h
	$(CC) $(CFLAGS) -c ../src/jpegenc.c -o jpegenc_core.o

test: jpegenc c_api_test
	./c_api_test
	./jpegenc cxx_api_test.jpg
	test -s cxx_api_test.jpg

clean:
	rm -rf *.o jpegenc c_api_test cxx_api_test.jpg
