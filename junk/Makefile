all: main


# /usr/lib/x86_64-linux-gnu/libOpenCL.so.1.0.0
main.o: main.cc
	gcc -c main.cc -std=c++0x

main: main.o
	# g++ main.o -o main -L/usr/lib/x86_64-linux-gnu -lOpenCL -lstdc++
	g++ main.o -o main /usr/lib/x86_64-linux-gnu/libOpenCL.so.1.0.0 -lstdc++

