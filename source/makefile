APP=drunner

INC=-Isource -Ibuildnum -Isource/tests -Ideps \
	-Isource/plugins -Isource/sourceplugins -Ideps/lua

CXXFLAGS=-D VERSION_STR="\"$(VCOMBO)\"" -D VERSION_STR_SHORT="\"$(VSHORT)\"" \
-D VERSION_NICEVER="\"$(NICEVER)\"" \
 $(DEBUG) -MD -MP -fdiagnostics-color=always \
 -Wall -Wno-unknown-pragmas -std=c++11

CFLAGS=-D LUA_USE_POSIX -MD -MP -fdiagnostics-color=always \
-Wall -Wno-unknown-pragmas

LDFLAGS=
LDLIBS=-lPocoFoundation -lPocoUtil

# On OS X we dynamically link runtime libs.
# On Linux we statically link runtime libs.
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
   CXXFLAGS += -static-libstdc++ -static-libgcc
   CCFLAGS += -static-libgcc
   LDFLAGS += -static-libstdc++ -static-libgcc -pthread
endif

# The default search paths on OS X are unlikely to include where Poco has been installed.
# Add the default location used by Brew, at least.
ifeq ($(UNAME_S),Darwin)
   CXXFLAGS += -I/usr/local/include
   LDFLAGS += -L/usr/local/lib
endif

CXXFLAGS += $(INC)
CFLAGS += $(INC)

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
