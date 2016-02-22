APP=drunnerc
CC=gcc
#CXX=g++
CXX=./g++
RM=rm -f
CPPFLAGS=-g -Wall -std=c++11 $(BUILD_NUMBER_LDFLAGS)
LDFLAGS=-lboost_filesystem -lboost_system
LDLIBS=


SRCS=$(shell find . -maxdepth 1 -name "*.cpp")
OBJS=$(subst .cpp,.o,$(SRCS))

all: $(APP)

$(APP): permissions build_number.h $(OBJS) makefile
	$(CXX) $(LDFLAGS) -o $(APP) $(OBJS) $(LDLIBS)

depend: .depend

.depend: $(SRCS)
	rm -f ./.depend
	$(CXX) $(CPPFLAGS) -MM $^>>./.depend;

clean:
	$(RM) $(OBJS)

dist-clean: clean
	$(RM) *~ .depend

HDRS=$(shell find . -maxdepth 1 \( -name "*.h" ! -name "build_number.h" \) )
build_number.h: $(SRCS) $(HDRS) major_version
	@echo
	@echo Bumping build number..
	sh make_buildnum.sh

permissions:
	chmod 0644 $(SRCS) $(HDRS)
	chmod -R 0755 bin
	

include .depend

