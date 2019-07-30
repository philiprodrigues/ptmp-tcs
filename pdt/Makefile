#Makefile for ADJACENCY c++ programs.

CPPFLAG= -I $(CANVAS_ROOT_IO_INC) \
         -I $(ROOT_INC)

CXXFLAGS=-std=c++14 -Wall -Werror -pedantic
CXX=g++
LDFLAGS=$$(root-config --libs --cflags)

AdjacencyAlgorithms.o: AdjacencyAlgorithms.cpp
	$(CXX) $(CPPFLAG) $(CXXFLAGS) $(LDFLAGS) -c $< -o $@

TriggerCandidate.o: TriggerCandidate.cpp
	$(CXX) $(CPPFLAG) $(CXXFLAGS) $(LDFLAGS) -c $< -o $@

ModuleTrigger.o: ModuleTrigger.cpp
	$(CXX) $(CPPFLAG) $(CXXFLAGS) $(LDFLAGS) -c $< -o $@

%.o: %.cc
	$(CXX) $(CPPFLAG) $(CXXFLAGS) $(LDFLAGS) -o $*.o -c $*.cc
	$(CXX) $(CPPFLAG) $(CXXFLAGS) $(LDFLAGS) -o $* AdjacencyAlgorithms.o TriggerCandidate.o ModuleTrigger.o $*.o

all: AdjacencyAlgorithms.o TriggerCandidate.o ModuleTrigger.o $(patsubst %.cc, %.o, $(wildcard *.cc))

clean:
	rm -f $(wildcard *.o) $(patsubst %.cc, %, $(wildcard *.cc)) 