CXX = g++
CXXFLAGS = -Wall

Driver: Sally.o Driver.cpp
	$(CXX) $(CXXFLAGS) Sally.o Driver.cpp -o Driver -g

driver1: Sally.o driver1.cpp
	$(CXX) $(CXXFLAGS) Sally.o driver1.cpp -o driver1 -g

driver2: Sally.o driver2.cpp
	$(CXX) $(CXXFLAGS) Sally.o driver2.cpp -o driver2 -g

Sally.o: Sally.cpp Sally.h
	$(CXX) $(CXXFLAGS) -c Sally.cpp -g

clean:
	rm *~

run:
	./Driver

run1:
	./driver1

run2:
	./driver2
