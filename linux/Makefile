CFLAGS=-D__LINUX__ -Wall -O2 
ASAN_FLAGS=-fsanitize=address -fno-omit-frame-pointer -O1 -g -Werror=vla
LIBS = 

all: jpegenc

jpegenc: main.o JPEGENC.o jpegenc_core.o
	$(CC) main.o JPEGENC.o jpegenc_core.o $(LIBS) -o jpegenc

c_api_test: c_api_test.o jpegenc_core.o
	$(CC) c_api_test.o jpegenc_core.o $(LIBS) -o c_api_test

edge_mcu_test: edge_mcu_test.o jpegenc_core_asan.o
	$(CC) $(ASAN_FLAGS) edge_mcu_test.o jpegenc_core_asan.o $(LIBS) -o edge_mcu_test

yuv420p_test: yuv420p_test.o jpegenc_core_asan.o
	$(CC) $(ASAN_FLAGS) yuv420p_test.o jpegenc_core_asan.o $(LIBS) -o yuv420p_test

main.o: main.cpp
	$(CXX) $(CFLAGS) -c main.cpp

c_api_test.o: c_api_test.c
	$(CC) $(CFLAGS) -c c_api_test.c

edge_mcu_test.o: edge_mcu_test.c
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -c edge_mcu_test.c

yuv420p_test.o: yuv420p_test.c
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -c yuv420p_test.c

JPEGENC.o: ../src/JPEGENC.cpp ../src/JPEGENC.h
	$(CXX) $(CFLAGS) -c ../src/JPEGENC.cpp

jpegenc_core.o: ../src/jpegenc.c ../src/jpegenc.inl ../src/JPEGENC.h
	$(CC) $(CFLAGS) -c ../src/jpegenc.c -o jpegenc_core.o

jpegenc_core_asan.o: ../src/jpegenc.c ../src/jpegenc.inl ../src/JPEGENC.h
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -c ../src/jpegenc.c -o jpegenc_core_asan.o

test: jpegenc c_api_test edge_mcu_test yuv420p_test
	./c_api_test
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 ./edge_mcu_test
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 ./yuv420p_test
	./jpegenc cxx_api_test.jpg
	test -s cxx_api_test.jpg

clean:
	rm -rf *.o jpegenc c_api_test edge_mcu_test yuv420p_test cxx_api_test.jpg
