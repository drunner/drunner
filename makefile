APP=output/drunner-install
CC=gcc
#CXX=g++
CXX=deps/colorgcc/colorgcc.pl
RM=rm -f
INC=-Isource -Ibuildnum -Isource/settings -Isource/generators -Isource/tests -Ideps/catch

BOOSTSTATIC=-static -pthread
CPPFLAGS=-Wall -Wno-unknown-pragmas -std=c++11 $(BOOSTSTATIC) $(BUILD_NUMBER_LDFLAGS) $(INC)
LDFLAGS=-static
LDLIBS=-lboost_filesystem -lboost_system -lyaml-cpp

OBJECTS_DIR=objs
SRCS=$(shell find source -maxdepth 2 -name "*.cpp")
HDRS=$(shell find source -maxdepth 2 -name "*.h")
OBJS=$(patsubst source/%,$(OBJECTS_DIR)/%,$(SRCS:.cpp=.o))

all: $(APP)

$(APP): permissions buildnum/build_number.h $(OBJS)
	$(CXX) $(LDFLAGS) -o $(APP) $(OBJS) $(LDLIBS)

.depend: buildnum/build_number.h $(SRCS) $(HDRS)
	rm -f ./.depend
	$(CXX) $(CPPFLAGS) -MM $^ | sed 's#^\(.*:\)#$(OBJECTS_DIR)/\1#' >>./.depend;

include .depend

$(OBJECTS_DIR)/%.o: source/%.cpp
	$(CXX) $(CPPFLAGS) -c -o $@ $<

clean:
	$(RM) $(OBJS)

dist-clean: clean
	$(RM) *~ .depend

buildnum/build_number.h: $(SRCS) $(HDRS) buildnum/major_version
	@echo
	@echo Bumping build number..
	cd buildnum ; ./make_buildnum.sh

upload: $(APP)
	cd output ; s3upload drunner-install
	cd prereqs ; s3upload install_docker.sh

uploaddev: $(APP)
	cd output ; mkdir dev ; cp drunner-install dev ; s3upload dev/drunner-install

install: $(APP)
	$(APP) -v ~/temp

permissions:
	mkdir -p objs/settings objs/generators objs/tests output
	find . -type d -exec chmod 0755 {} \;
	find . -type f -exec chmod 0644 {} \;
	chmod 0755 buildnum/make_buildnum.sh pullall deps/colorgcc/colorgcc.pl
	touch permissions
