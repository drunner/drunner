APP=output/drunner
CPP=g++
INC=-Isource -Ibuildnum -Isource/tests -Ideps -Isource/plugins -Ideps/yaml-cpp/include -Ideps/lua

CXXFLAGS=-D VERSION_STR="\"$(VCOMBO)\"" -D VERSION_STR_SHORT="\"$(VSHORT)\"" \
 $(DEBUG) -MD -MP -fdiagnostics-color=always \
 -Wall -Wno-unknown-pragmas -static-libgcc -std=c++11 $(INC)

CFLAGS=-MD -MP -fdiagnostics-color=always -Wall -Wno-unknown-pragmas -static-libgcc $(INC)

LDFLAGS=-static -pthread
LDLIBS=-lPocoFoundation -lPocoUtil

APP=output/drunner

OBJECTS_DIR=objs
SRCS=$(shell find . -name "*.cpp")
OBJS=$(SRCS:.cpp=.o)

SRCSC=$(shell find . -name "*.c")
OBJSC=$(SRCSC:.c=.o)

all: $(APP)

$(APP): $(OBJS) $(OBJSC)
	$(CXX) $(LDFLAGS) -o $(APP) $(OBJS) $(OBJSC) $(LDLIBS)

-include $(SRCS:.cpp=.d)
-include $(SRCSC:.c=.d)

clean:
	rm -f $(OBJS) $(OBJSC)
