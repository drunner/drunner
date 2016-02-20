APP=drunnerc
CC=gcc
#CXX=g++
CXX=./g++
RM=rm -f
CPPFLAGS=-g -Wall -std=c++11
LDFLAGS=-lboost_filesystem -lboost_system 
LDLIBS=

SRCS=$(shell find . -maxdepth 1 -name "*.cpp")
OBJS=$(subst .cpp,.o,$(SRCS))

all: $(APP)

$(APP): $(OBJS) makefile
	$(CXX) $(LDFLAGS) -o $(APP) $(OBJS) $(LDLIBS)

depend: .depend

.depend: $(SRCS)
	rm -f ./.depend
	$(CXX) $(CPPFLAGS) -MM $^>>./.depend;

clean:
	$(RM) $(OBJS)

dist-clean: clean
	$(RM) *~ .depend

include .depend
