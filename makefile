APP=bin/drunner-install
CC=gcc
#CXX=g++
CXX=bin/colorgcc.pl
RM=rm -f
INC=-Isource -Ibuildnum

CPPFLAGS=-g -Wall -std=c++11 $(BUILD_NUMBER_LDFLAGS) $(INC)
LDFLAGS=-lboost_filesystem -lboost_system
LDLIBS=

SRCS=$(shell find source -maxdepth 1 -name "*.cpp")
OBJS=$(patsubst source/%,objs/%,$(SRCS:.cpp=.o))

all: buildnum/build_number.h $(APP)

$(APP): $(OBJS)
	$(CXX) $(LDFLAGS) -o $(APP) $(OBJS) $(LDLIBS)

objs/%.o: source/%.cpp
	$(CXX) $(CPPFLAGS) -c -o $@ $<


depend: .depend

.depend: $(SRCS)
	rm -f ./.depend
	$(CXX) $(CPPFLAGS) -MM $^>>./.depend;

clean:
	$(RM) $(OBJS)

dist-clean: clean
	$(RM) *~ .depend

buildnum/build_number.h: $(SRCS) buildnum/major_version
	@echo
	@echo Bumping build number..
	buildnum/make_buildnum.sh

permissions:
	chmod 0644 $(SRCS) $(HDRS) buildnum/* source/*
	chmod 0755 bin/* buildnum buildnum/make_buildnum.sh objs source	

include .depend

upload: $(APP)
	cd bin/ ; upload drunner-install

push: $(APP)
	git pull ; git add . ; git commit ; git push
