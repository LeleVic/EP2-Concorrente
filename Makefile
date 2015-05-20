ep2: main.o math_2.o thread.o
	g++-4.7 -pthread -std=c++11 main.o math_2.o thread.o -o ep2 -lgmpxx -lgmp

main.o: main.cpp math_2.h thread.h main.h
	g++-4.7 -Wall -pthread -std=c++11 -c main.cpp 

thread.o: thread.cpp thread.h math_2.h
	g++-4.7 -Wall -pthread -std=c++11 -c thread.cpp

math_2.o: math_2.cpp math_2.h
	g++-4.7 -Wall -std=c++11 -c math_2.cpp


.PHONY: clean
clean:
	rm -f *.o