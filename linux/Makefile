CFLAGS=-D__LINUX__ -Wall -O2 
LIBS = 

all: jpegenc

jpegenc: main.o JPEGENC.o
	$(CC) main.o JPEGENC.o $(LIBS) -o jpegenc 

main.o: main.cpp
	$(CXX) $(CFLAGS) -c main.cpp

JPEGENC.o: ../src/JPEGENC.cpp ../src/jpegenc.inl ../src/JPEGENC.h
	$(CXX) $(CFLAGS) -c ../src/JPEGENC.cpp

clean:
	rm -rf *.o jpegenc
