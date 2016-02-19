APP=drunnerc
CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS=-g -Wall
LDFLAGS=
LDLIBS=

SRCS=$(shell find . -maxdepth 1 -name "*.cpp")
OBJS=$(subst .cc,.o,$(SRCS))

all: $(APP)

$(APP): $(OBJS)
	$(CXX) $(LDFLAGS) -o tool $(OBJS) $(LDLIBS) 

depend: .depend

.depend: $(SRCS)
	rm -f ./.depend
	$(CXX) $(CPPFLAGS) -MM $^>>./.depend;

clean:
	$(RM) $(OBJS)

dist-clean: clean
	$(RM) *~ .depend

include .depend
