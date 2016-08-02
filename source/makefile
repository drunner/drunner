APP=drunner
CPP=g++
INC=-Isource -Ibuildnum -Isource/tests -Ideps -Isource/plugins -Ideps/lua

CXXFLAGS=-D VERSION_STR="\"$(VCOMBO)\"" -D VERSION_STR_SHORT="\"$(VSHORT)\"" \
 $(DEBUG) -MD -MP -fdiagnostics-color=always \
 -Wall -Wno-unknown-pragmas -std=c++11 $(INC)

CFLAGS=-D LUA_USE_POSIX -MD -MP -fdiagnostics-color=always -Wall -Wno-unknown-pragmas $(INC)

LDFLAGS=-pthread
LDLIBS=-lPocoFoundation -lPocoUtil

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
	rm -f $(SRCS:.cpp=.d) $(SRCSC:.c=.d)
