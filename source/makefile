APP=output/drunner
CPP=g++
INC=-Isource -Ibuildnum -Isource/tests -Ideps -Isource/plugins -Ideps/yaml-cpp/include
CPPFLAGS=-D VERSION_STR="\"$(VCOMBO)\"" -D VERSION_STR_SHORT="\"$(VSHORT)\"" \
 $(DEBUG) -MD -MP -fdiagnostics-color=always \
 -Wall -Wno-unknown-pragmas -static-libgcc -std=c++11 $(INC)
LDFLAGS=-static -pthread
LDLIBS=-lPocoFoundation -lPocoUtil

APP=output/drunner

OBJECTS_DIR=objs
SRCS=$(shell find . -name "*.cpp")
OBJS=$(SRCS:.cpp=.o)

all: $(APP)

$(APP): $(OBJS)
	$(CXX) $(LDFLAGS) -o $(APP) $(OBJS) $(LDLIBS)

-include $(SRC:.cpp=.d)

%.o: %.cpp
	$(CPP) $(CPPFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS)
